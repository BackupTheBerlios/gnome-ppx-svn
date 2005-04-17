#ifndef __PPP_MANAGER_H__
#define __PPP_MANAGER_H__
#include <glib.h>
#include <glib-object.h>
#include <pppd.h>

G_BEGIN_DECLS

#define PPP_TYPE_MANAGER            (ppp_manager_get_type())
#define PPP_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PPP_TYPE_MANAGER, PppManager))
#define PPP_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PPP_TYPE_MANAGER, PppManagerClass))
#define PPP_IS_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPP_TYPE_MANAGER))
#define PPP_IS_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),PPP_TYPE_MANAGER))
#define PPP_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PPP_TYPE_MANAGER, PppManagerClass))

typedef struct _PppManager PppManager;
typedef struct _PppManagerClass PppManagerClass;
#include "ppp_dialer.h"

/* Object structure */
struct _PppManagerClass {
	GObjectClass parent;
	/* signal called when user clicks about box, @origin is the widget which the
	 * user used to request an about dialog.
	 */
	void (*about) (PppManager *self, GtkWidget *origin);
};

struct _PppManager {
	GObject parent;
};

GType ppp_manager_get_type ();

PppManager* ppp_manager_new (const gchar *glade_file, const gchar *link_name, const gchar *key_use_tray);
const gchar *ppp_manager_get_link_name (PppManager * self);
const gchar *ppp_manager_get_glade_file (PppManager * self);
const gchar *ppp_manager_get_gconf_dir (PppManager * self);
void ppp_manager_set_dialer (PppManager * self, PppDialer *dialer);
PppDialer *ppp_manager_get_dialer (PppManager * self);
pppd_monitor* ppp_manager_get_monitor (PppManager * self);
/*
 * Start a new connection 
 */
gboolean ppp_manager_call (PppManager * self, const gchar *pppd, const gchar *username, GSList *opts);
/* tries to monitor an existing connection */
gboolean ppp_manager_run (PppManager * self);
/* disconnects an ongoing connection. if 'confirm' is set to TRUE then a dialog will be
 * shown asking the user for confirmation
 */
gboolean ppp_manager_disconnect (PppManager *self, gboolean confirm);
/*
 * Quits the connection dialer without disconnecting.
 */
gboolean ppp_manager_quit (PppManager *self, gboolean confirm);
/*
 * Tests if user wants to use tray icon.
 */
gboolean ppp_manager_use_tray (PppManager *self);

/*
 * Returns the connection log
 */
GSList *ppp_manager_get_log (PppManager *self);
G_END_DECLS

#endif /* __PPP_MANAGER_H__ */
