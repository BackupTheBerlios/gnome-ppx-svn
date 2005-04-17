/*
 * Protected class.
 * Ppp::Status shows details and statistics about the connection.
 * It extends a Gtk::Window.
 */ 
#ifndef __PPP_STATUS_H__
#define __PPP_STATUS_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "ppp_manager.h"

G_BEGIN_DECLS

#define PPP_TYPE_STATUS            (ppp_status_get_type())
#define PPP_STATUS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PPP_TYPE_STATUS, PppStatus))
#define PPP_STATUS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PPP_TYPE_STATUS, PppStatusClass))
#define PPP_IS_STATUS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPP_TYPE_STATUS))
#define PPP_IS_STATUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),PPP_TYPE_STATUS))
#define PPP_STATUS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PPP_TYPE_STATUS, PppStatusClass))

/* Object/Class definition */
typedef struct _PppStatus PppStatus;
typedef struct _PppStatusClass PppStatusClass;

/* Object/Class structures */
struct _PppStatusClass {
	GtkWindowClass parent_class;
};

struct _PppStatus {
	GtkWindow parent;
};

GType ppp_status_get_type ();

/* creates a new status object */
GtkWidget *ppp_status_new (PppManager *manager);

G_END_DECLS

#endif /* __PPP_STATUS_H__ */
