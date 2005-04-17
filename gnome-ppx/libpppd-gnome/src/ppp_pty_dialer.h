#ifndef __PPP_PTY_DIALER_H__
#define __PPP_PTY_DIALER_H__

#include <glib.h>
#include <glib-object.h>
#include "ppp_dialer.h"

/*
 * This is a particular implementation for pty_dialers that only
 * use the 'pty' directive. It already defines a set of default
 * options: call <account> promptprog $libexec_dir/ppp_get_password pty <value>
 * It checks for the application on startup and doesn't let the pty_dialer start
 * when that happens.
 * Classes that intend to extend this one must implement the protected method
 * gen_pty which must return a newly allocated pty entry which will be posteriorly
 * free'd with g_free.
 * User has to implement the constructor for defining the GUI
 * Implemented protected methods: get_options, can_connect
 */
G_BEGIN_DECLS

#define PPP_TYPE_PTY_DIALER            (ppp_pty_dialer_get_type())
#define PPP_PTY_DIALER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PPP_TYPE_PTY_DIALER, PppPtyDialer))
#define PPP_PTY_DIALER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PPP_TYPE_PTY_DIALER, PppPtyDialerClass))
#define PPP_IS_PTY_DIALER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPP_TYPE_PTY_DIALER))
#define PPP_IS_PTY_DIALER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),PPP_TYPE_PTY_DIALER))
#define PPP_PTY_DIALER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PPP_TYPE_PTY_DIALER, PppPtyDialerClass))

/* Object/Class definition */
typedef struct _PppPtyDialer PppPtyDialer;
typedef struct _PppPtyDialerClass PppPtyDialerClass;

/* Object structure */
struct _PppPtyDialerClass {
	PppDialerClass parent;
	/* protected methods */
	gchar* (*gen_pty) (PppPtyDialer *self);
};

struct _PppPtyDialer {
	PppDialer parent;
};

GType ppp_pty_dialer_get_type ();
/* returns the application's full path */
gchar *ppp_pty_dialer_get_application_path (PppPtyDialer *self);

G_END_DECLS

#endif /* __PPP_PTY_DIALER_H__ */
