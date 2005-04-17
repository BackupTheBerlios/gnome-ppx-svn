noinst_LIBRARIES = libpppd.a
noinst_PROGRAMS = test_pppd_monitor
EXTRA_DIST = pppd.sh

libpppd_a_SOURCES = \
	pppd_monitor.c pppd_monitor.h \
	pppd_parser.c pppd_parser.h \
	pppd_util.c pppd_util.h \
	pppd.h

test_pppd_monitor_LDFLAGS = -pthread libpppd.a
test_pppd_monitor_SOURCES = test_pppd_monitor.c
