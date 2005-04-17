AM_CFLAGS  = @PPPD_GNOME_CFLAGS@
AM_LDFLAGS = @PPPD_GNOME_LIBS@ 

EXTRA_DIST = ppp.marshallers
noinst_LIBRARIES = libpppd_gnome.a
libexec_PROGRAMS  = ppp_get_password

libpppd_gnome_a_SOURCES = \
	netstatus-sysdeps.c netstatus-sysdeps.h \
	ppp_status.h ppp_status.c \
	ppp_progress.h ppp_progress.c \
	ppp_marshallers.h ppp_marshallers.c \
	ppp_dialer.c ppp_dialer.h \
	ppp_manager.c ppp_manager.h \
	ppp_tray.c ppp_tray.h \
	pppd_gnome.h \
	ppp_state_listener.c ppp_state_listener.h \
	ppp_pty_dialer.c ppp_pty_dialer.h

ppp_marshallers.h: ppp.marshallers
	(cd $(srcdir) && \
	@GLIB_GENMARSHAL@ --prefix=ppp_marshallers ppp.marshallers --header) > xgen-gmh \
	&& (cmp -s xgen-gmh ppp_marshallers.h || cp xgen-gmh ppp_marshallers.h) \
	&& rm -f xgen-gmh xgen-gmh~

ppp_marshallers.c: ppp.marshallers
	(cd $(srcdir) && \
	@GLIB_GENMARSHAL@ --prefix=ppp_marshallers ppp.marshallers --body) > xgen-gmc \
	&& cp xgen-gmc ppp_marshallers.c \
	&& rm -f xgen-gmc xgen-gmc~

ppp_get_password_SOURCES = ppp_get_password.c

BUILT_SOURCES = ppp_marshallers.c ppp_marshallers.h

CLEANFILES = ppp_marshallers.c ppp_marshallers.h

