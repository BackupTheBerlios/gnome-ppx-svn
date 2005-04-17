/*
 * Monitors the state of a pppd connection. You can use it to track an
 * existing connection or to monitor a new one. The ppp options are sent
 * directly through 'pppd_call' function. It is also possible to setup a
 * state listener which is a callback function which will be called whenever
 * the state changes, not that this call is performed from within a monitor
 * thread.
 * To setup a monitor you need to specify a link name, this is a unique
 * name that identifies a connection, please consult 'pppd' manual for
 * further info.
 */
#ifndef __PPPD_MONITOR_H__
#define __PPPD_MONITOR_H__
#include "pppd_util.h"
/*
 * Machine state
 * IDLE -> CONNECTING
 * CONNECTING -> CONNECTED
 * CONNECTED -> IDLE
 */
typedef enum {
	PPPD_MONITOR_IDLE         = 1 << 1,
	PPPD_MONITOR_CONNECTING   = 1 << 2,
	PPPD_MONITOR_CONNECTED    = 1 << 3,
} pppd_monitor_state;

typedef enum {
	PPPD_MONITOR_DETACHED,
	PPPD_MONITOR_ERROR,
	PPPD_MONITOR_DISCONNECTED,
} pppd_monitor_reason;

typedef struct _pppd_monitor pppd_monitor;

struct _pppd_monitor_callbacks {
	void (*on_state_changed) (pppd_monitor *self, void *userdata);
	/* returns the PID, must be grater then 0, otherwise it's considered an error */
	int  (*exec) (char * const argv[], void *userdata);
	/* kills a given pid */
	int  (*kill) (int pid, void *userdata);
};

typedef struct _pppd_monitor_callbacks pppd_monitor_callbacks;


/* creates a new pppd monitor */
pppd_monitor *pppd_monitor_new (const char *linkname, void *listener_user_data, pppd_monitor_callbacks *cb);
/* frees allocated resource. The monitor *must* be in IDLE state. */
void pppd_monitor_free (pppd_monitor *self);
/* stops the associated connection, only works when it is in CONNECTING or
 * CONNECTED state. It's blocked until it succeeds. Since it stops the monitor
 * after this call the monitor is in IDLE state.
 */
int pppd_monitor_abort (pppd_monitor *self, void *user_data);
/* returns this monitor's state */
pppd_monitor_state pppd_monitor_get_state (pppd_monitor *self);
/* waits for a certain event or events, they can be ORed 
 * a call to resume must be done before continuing with any other procedure.
 */
void pppd_monitor_pause (pppd_monitor *self, pppd_monitor_state state);
void pppd_monitor_resume (pppd_monitor *self);
/*
 * dials the given connection, it is called asynchronously, if there is 
 * a connection in course the calling thread is blocked until it finishes.
 * Returns the success (1) or not (0) of the action.
 */
int pppd_monitor_call (pppd_monitor *self, const char *pppd, char * const argv[], void *user_data);
/*
 * stops monitoring the connection, leaves it intact, not disconnecting it.
 * the monitor dies and is set to IDLE. DO NOT CALL THIS FUNCTION INSIDE PAUSE/RESUME BLOCKS!
 */
void pppd_monitor_detach (pppd_monitor *self);
/*
 * attaches this monitor to an already running link connection.
 * This only starts the monitor, it does not wait for it to end.
 */
int pppd_monitor_attach (pppd_monitor *self);
/*
 * This only has a value if the state is CONNECTED, otherwise
 * it is returned NULL.
 */
const char *pppd_monitor_get_interface (pppd_monitor *self);
/*
 * Returns the time the connection has started.
 */
time_t pppd_monitor_get_time (pppd_monitor *self);

/*
 * Returns the reason why monitor stopped working.
 * Should only be used after used an in IDLE state.
 * Otherwise the results are of the last monitor result,
 * in case of never working it will return DETACHED.
 */
pppd_monitor_reason pppd_monitor_get_reason (pppd_monitor *self);
/* returns the monitored interface IP and Netmask, only a valid value when we are in CONNECTED state */
void pppd_monitor_get_ip (pppd_monitor *self, unsigned char address[4], unsigned char netmask[4]);
/* returns the associated link name */
const char *ppp_monitor_get_link_name (pppd_monitor *self);
#endif /* __PPPD_MONITOR_H__ */
