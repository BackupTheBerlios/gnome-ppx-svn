#include "pppd_monitor.h"
#include <stdio.h>

/*
 Tested so far:
 call, pause, resume, detach, attach, abort
 */
static void listener (pppd_monitor *m)
{
	printf("New state: %d\n", pppd_monitor_get_state(m));
}

int main () {
	char *linkname = "pppoe";
	pppd_option opt_call, opt_pty, opt_promptprog;
	pppd_monitor *m;
	char *iface = NULL;
	int pid = 0;
//	m = pppd_monitor_new(linkname, NULL, NULL);
	PPPD_OPT_SET_STR(opt_call, "call", "pppoe");
	PPPD_OPT_SET_STR(opt_pty, "pty", "/usr/sbin/pppoe -I nas0");
	PPPD_OPT_SET_STR(opt_promptprog, "promptprog", "/usr/bin/pppoe-get-password");
	{
		pppd_option *opts[] = {
			&opt_call,
			&opt_pty,
			&opt_promptprog,
			NULL
		};
		char *pppd = "/usr/sbin/pppd";
		//pppd = "/home/tiago/Projects/tmp/localhost/gnome-ppx/libpppd/src/pppd.sh";
		/*
		if (!pppd_monitor_attach(m) && !pppd_monitor_call (m, pppd, "username", opts)) {
			puts("could not create connection!");
	puts("fdasd");
			exit(1);
		}
		*/
	}
	puts("done, now wait");
	sleep(3);
	pppd_monitor_abort(m, NULL);
	sleep(2);
	/* now we wait for a certain state */
	/* pppd_monitor_abort(m); */
	pppd_monitor_detach(m);
	
	pppd_monitor_pause (m, PPPD_MONITOR_IDLE);
	puts("we are offline :(");
	pppd_monitor_resume (m);
	pppd_monitor_free (m);
	
	/* ok here we are certain the requested event is reached. */
	puts("done!");
	return 0;
}
