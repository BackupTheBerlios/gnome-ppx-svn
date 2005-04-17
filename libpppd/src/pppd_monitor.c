#include "pppd_monitor.h"
#include "pppd_util.h"
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>

#define WAITING_TIME 250000
struct _pppd_monitor {
	pppd_monitor_state state;
	pppd_monitor_callbacks *cb;
	void * listener_data;
	char * linkname;
	char iface[IFNAMSIZ];
	int pid;
	time_t time;
	unsigned char address[4];
	unsigned char netmask[4];
	pppd_monitor_reason reason;
	/* enforces the access to a
	 * state change.
	 */
	pthread_mutex_t state_mutex;
	/* a semaphore that allows
	 * state listeners to grab the next
	 * state
	 */
	pthread_cond_t state_cond;
	/* a mutex that allows only one
	 * monitor running per instance.
	 */
	pthread_mutex_t monitor_mutex;
	/* condition flag, which represents 
	 * the order of disconnecting
	 */
	int abort;
	pthread_cond_t abort_cond;
	/*
	 * Detached, semaphore, and mutex
	 */
	int detach;
	pthread_cond_t detach_cond;
};

static int pppd_monitor_update_ip (pppd_monitor *self);

pppd_monitor *pppd_monitor_new (const char *linkname, void *listener_user_data, pppd_monitor_callbacks *cb)
{
	pppd_monitor *self;
	assert (cb);
	assert (linkname);
	self = malloc(sizeof(pppd_monitor));
	assert(self);
	self->detach = 0;
	self->abort = 0;
	self->linkname = strdup(linkname);
	self->time = -1;
	pthread_mutex_init(&self->state_mutex, NULL);
	pthread_mutex_init(&self->monitor_mutex, NULL);
	pthread_cond_init(&self->state_cond,NULL);
	pthread_cond_init(&self->abort_cond,NULL);
	pthread_cond_init(&self->detach_cond,NULL);
	self->listener_data = listener_user_data;
	self->state = PPPD_MONITOR_IDLE;
	self->reason = PPPD_MONITOR_DETACHED;
	self->cb = cb;
	return self;
}

void pppd_monitor_free(pppd_monitor *self)
{
	assert(self->state == PPPD_MONITOR_IDLE);
	free (self->linkname);
	pthread_mutex_destroy(&self->state_mutex);
	pthread_mutex_destroy(&self->monitor_mutex);
	pthread_cond_destroy(&self->state_cond);
	pthread_cond_destroy(&self->abort_cond);
	pthread_cond_destroy(&self->detach_cond);
	free (self);
}

/*
 * changing states takes care of warning the semaphores and
 * calling the listener. When the monitor is set to IDLE 
 * state it resets the flags, if the abort command was issued
 * a signal is sent to the semaphore which blocks the call.
 * It also free's the allocated resources like the interface
 * and resets the PID.
 */
static void pppd_monitor_set_state (pppd_monitor *self, pppd_monitor_state state)
{
	/* block the state change */
	pthread_mutex_lock(&self->state_mutex);
	assert (self->state != state);
	/* change the state */
	self->state = state;
	
	/* signal waiting thread */
	pthread_cond_signal(&self->state_cond);
	
	/* run callback */
	if (self->cb->on_state_changed)
		self->cb->on_state_changed(self, self->listener_data);
	
	/* we allow monitor to go on */
	if (state == PPPD_MONITOR_IDLE) {
		/* we also reset the interface and pid if they were set */
		self->iface[0] = '\0';
		self->pid = -1;
		/* we can't be aborting since it's IDLE */
		if (self->abort) {
			/* unblocks the abort call */
			pthread_cond_signal(&self->abort_cond);
		}
	}
	
	pthread_mutex_unlock(&self->state_mutex);
}

pppd_monitor_state pppd_monitor_get_state (pppd_monitor *self)
{
	assert(self);
	return self->state;
}

void pppd_monitor_pause (pppd_monitor *self, pppd_monitor_state state)
{
	assert(self);
	/* we lock the state mutex, to make sure it's not changed
	 * until we reach the loop.
	 */
	pthread_mutex_lock(&self->state_mutex);
	/* we are waiting until the monitor was stopped
	 * or if we didn't reach our state.
	 */
	while ((state == PPPD_MONITOR_IDLE || self->state < state) && self->state != PPPD_MONITOR_IDLE) {
		/* when we wait the mutex is unlocked */
		pthread_cond_wait(&self->state_cond, &self->state_mutex);
	}
	
	/* the unlock must be called. */
}

void pppd_monitor_resume (pppd_monitor *self)
{
	assert(self);
	/* finaly release the lock */
	pthread_mutex_unlock (&self->state_mutex);
}

static void *pppd_monitor_probe (void *data)
{
	pppd_monitor *self = (pppd_monitor *) data;
	int status;
	/* if we are connecting, let's wait until we are connected */
	if (self->state == PPPD_MONITOR_CONNECTING) {
		/* monitor connecting state */
		waitpid(self->pid, &status, 0);
		/* reset pid */
		self->pid = -1;
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 || !pppd_get_link(self->linkname, &self->pid, self->iface, &self->time) || !pppd_monitor_update_ip(self)) {
			pppd_monitor_set_state(self, PPPD_MONITOR_IDLE);
			self->reason = self->abort ? PPPD_MONITOR_DISCONNECTED : PPPD_MONITOR_ERROR;
			pthread_exit(NULL);
			return NULL;
		}
	}
	/* link is up! */
	pppd_monitor_set_state(self, PPPD_MONITOR_CONNECTED);
	/* probe it until is not */
	while (!self->detach && !kill(self->pid, 0))
		usleep(WAITING_TIME);
	if (self->detach) {
		pthread_cond_signal(&self->detach_cond);
	} else {
		/* we are not going to detach */
		self->reason = PPPD_MONITOR_DISCONNECTED;
	}
	pppd_monitor_set_state (self, PPPD_MONITOR_IDLE);
	pthread_exit(NULL);
	return NULL;
}

int pppd_monitor_call (pppd_monitor *self, const char *pppd, char * const argv[], void *user_data)
{
	pthread_t t;
	
	int argc;
	char **new_argv;
	
	assert(self && pppd && argv);
	/* we assure only one call */
	pthread_mutex_lock(&self->monitor_mutex);
	/* first we wait for idle state */
	pthread_mutex_lock(&self->state_mutex);
	while (self->state != PPPD_MONITOR_IDLE)
		pthread_cond_wait(&self->state_cond, &self->state_mutex);
	pthread_mutex_unlock(&self->state_mutex);
	
	for (argc = 0; argv[argc]; argc++)
		/* count the number of options */;
	
	/* we have to append an extended option called 'link' */
	new_argv = malloc (argc * (sizeof(char *) + 3));
	assert(new_argv);
	/* copy old contents */
	memcpy(new_argv + 3, argv, argc * sizeof(char *));
	/* now append our last option */
	new_argv[0] = (char *)pppd;
	new_argv[1] = "linkname";
	new_argv[2] = self->linkname;

	self->pid = self->cb->exec (argv, user_data);
	if (self->pid <= 0) {
		free (new_argv);
		pthread_mutex_unlock(&self->monitor_mutex);
		return 0;
	}
	/* we don't need it anymore */
	free (new_argv);
	
	/* we are now connecting */
	pppd_monitor_set_state(self, PPPD_MONITOR_CONNECTING);
	/* monitor link state */
	pthread_create(&t, NULL, pppd_monitor_probe, self);
	/* all went fine */
	pthread_mutex_unlock(&self->monitor_mutex);
	return 1;
}

void pppd_monitor_detach (pppd_monitor *self)
{
	/* when 2 simultanious calls are done
	 * to this function, because state is
	 * locked, one waits for the other to
	 * end, when the 2nd reaches the critical
	 * zone it finds the the state is IDLE
	 * and returns, therefore no 2 calls are
	 * issued.
	 */
	pthread_mutex_lock(&self->state_mutex);
	/* first let's check if we are working */
	if (self->state == PPPD_MONITOR_IDLE) {
		pthread_mutex_unlock(&self->state_mutex);
		return;
	}
	self->detach = 1;
	while (self->state != PPPD_MONITOR_IDLE) {
		/* unlock for next state */
		pthread_cond_wait(&self->detach_cond,&self->state_mutex);
	}
	/* ok we are done */
	self->detach = 0;
	/* set why monitor stopped */
	self->reason = PPPD_MONITOR_DETACHED;
	pthread_mutex_unlock(&self->state_mutex);
}

int pppd_monitor_abort (pppd_monitor *self, void *user_data)
{
	int ret;
	/* when 2 simultanious calls are done
	 * to this function, because state is
	 * locked, one waits for the other to
	 * end, when the 2nd reaches the critical
	 * zone it finds the the state is IDLE
	 * and returns, therefore no 2 calls are
	 * issued.
	 */
	pthread_mutex_lock(&self->state_mutex);
	/* first let's check if we are working */
	if (self->state == PPPD_MONITOR_IDLE) {
		pthread_mutex_unlock(&self->state_mutex);
		return 1;
	}
	self->abort = 1;
	ret = self->cb->kill (self->pid, user_data);
	/* only wait for next state if we could actually kill it */
	if (ret) {
		while (self->state != PPPD_MONITOR_IDLE) {
			/* unlock for next state */
			pthread_cond_wait(&self->abort_cond, &self->state_mutex);
		}
		/* ok we are done */
		self->abort = 0;
	}
	pthread_mutex_unlock(&self->state_mutex);
	return ret;
}

int pppd_monitor_attach (pppd_monitor *self)
{
	pthread_t t;
	/* wait until the monitor stops running */
	pthread_mutex_lock(&self->monitor_mutex);
	/* now we must try to get the link and check if it's running */
	if (!pppd_get_link(self->linkname, &self->pid, self->iface, &self->time) || !pppd_monitor_update_ip(self)) {
		pthread_mutex_unlock(&self->monitor_mutex);
		return 0;
	}
	/* check if pppd is there */
	if (kill(self->pid, 0)) {
		pthread_mutex_unlock(&self->monitor_mutex);
		return 0;
	}
	/* everything fine let's launch the monitor */
	pthread_create(&t, NULL, pppd_monitor_probe, self);
	pthread_mutex_unlock(&self->monitor_mutex);
	return 1;
}

const char *pppd_monitor_get_interface (pppd_monitor *self)
{
	return self->iface;
}

time_t pppd_monitor_get_time (pppd_monitor *self)
{
	return self->time;
}

static int pppd_get_ip (const char *iface, int fd, int id, char addr[4])
{
	struct ifreq ifr;
	struct sockaddr_in *s_addr;
		
	strcpy(ifr.ifr_name, iface);
	if (ioctl(fd, id, &ifr) < 0) {
		return 0;
	}
	s_addr = (struct sockaddr_in *) &ifr.ifr_addr;
	
	memcpy(addr, &(s_addr->sin_addr), sizeof(addr));
	return 1;
}

static int pppd_monitor_update_ip (pppd_monitor *self)
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	int ret = 1;
	
	if (fd < 0)
		return 0;
	
	ret &= pppd_get_ip(self->iface, fd, SIOCGIFADDR, self->address);
	ret &= pppd_get_ip(self->iface, fd, SIOCGIFNETMASK, self->netmask);
	close(fd);
	return ret;
}

void pppd_monitor_get_ip (pppd_monitor *self, unsigned char address[4], unsigned char netmask[4])
{
	memcpy(address, self->address, sizeof(address));
	memcpy(netmask, self->netmask, sizeof(netmask));
}

pppd_monitor_reason pppd_monitor_get_reason (pppd_monitor *self)
{
	return self->reason;
}

const char *pppd_monitor_get_link_name (pppd_monitor *self)
{
	return self->linkname;
}
