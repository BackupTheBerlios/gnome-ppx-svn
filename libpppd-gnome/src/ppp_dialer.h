/*
 * Because Ppp::Dialer is a Ppp::EventListener implementations of 
 * Ppp::Dialer can override Ppp::EventListener.on_startup() to 
 * check in runtime if the dialer can start or not (eg. to check a program).
 * For example, this  procedure is used with Pppoe::Dialer and Pptp::Dialer 
 * to not allow the Dialer to start if it cannot find the needed programs 
 * (namely pppoe and pptp).
 */
#ifndef __PPP_DIALER_H__
#define __PPP_DIALER_H__
#include <glib.h>
#include <glib-object.h>
#include <gconf/gconf-client.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PPP_TYPE_DIALER            (ppp_dialer_get_type())
#define PPP_DIALER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PPP_TYPE_DIALER, PppDialer))
#define PPP_DIALER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PPP_TYPE_DIALER, PppDialerClass))
#define PPP_IS_DIALER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPP_TYPE_DIALER))
#define PPP_IS_DIALER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),PPP_TYPE_DIALER))
#define PPP_DIALER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PPP_TYPE_DIALER, PppDialerClass))
/* Object/Class definition */
typedef struct _PppDialer PppDialer;
typedef struct _PppDialerClass PppDialerClass;
#include "ppp_manager.h"

/* Object structure */
struct _PppDialerClass {
	GtkWindowClass parent;
	/* protected abstract methods 
	 * users are not enforced to implement these methods
	 * not implementing any of this (setting it to NULL)
	 * makes them not be called.
	 */
	
	/* this is called after the GUI is setup, inside the constructor */
	void     (*gui_init)             (PppDialer *self);
	/* method called when dialer intends to save changes */
	void     (*save_changes)         (PppDialer *self);
	/* method called when dialer wants to get calling options */
	GSList*  (*get_options)          (PppDialer* self);
	/* method called to check if connect button should be sensitive
	 * note: it will only be connect if the username and password are
	 * filled.
	 */
	gboolean (*connect_is_sensitive) (PppDialer* self);
	/* method called after user clicks the connect button 
	 * and before actually running pppd
	 */
	gboolean (*can_connect)          (PppDialer* self);
};

struct _PppDialer {
	GtkWindow parent;
};

GType ppp_dialer_get_type ();

GtkTable *ppp_dialer_get_table(PppDialer *self);
GtkWidget *ppp_dialer_get_preferences_button (PppDialer *self);
PppManager *ppp_dialer_get_manager (PppDialer *self);
/* updates connect button's sensitivity */
void ppp_dialer_update_connect_sensitivity (PppDialer *self);
void ppp_dialer_set_title (PppDialer *self, const gchar *title);

/* if @app_name is NULL it will be found */
gboolean ppp_dialer_check_app (const gchar *gconf_key, const gchar *app_name);

G_END_DECLS

#endif /* __PPP_DIALER_H__ */
