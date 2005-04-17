#ifndef __PPPOE_DIALER_H__
#define __PPPOE_DIALER_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <pppd_gnome.h>

G_BEGIN_DECLS

#define PPPOE_TYPE_DIALER            (pppoe_dialer_get_type())
#define PPPOE_DIALER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PPPOE_TYPE_DIALER, PppoeDialer))
#define PPPOE_DIALER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PPPOE_TYPE_DIALER, PppoeDialerClass))
#define PPPOE_IS_DIALER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPPOE_TYPE_DIALER))
#define PPPOE_IS_DIALER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),PPPOE_TYPE_DIALER))
#define PPPOE_DIALER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PPPOE_TYPE_DIALER, PppoeDialerClass))
#define PPPOE_BASE_KEY               "/apps/gnome-pppoe"
#define PPPOE_KEY_INTERFACE          PPPOE_BASE_KEY "/interface"
#define PPPOE_KEY_PPPOE_PATH         PPPOE_BASE_KEY "/pppoe_path"
#define PPPOE_KEY_USE_TRAY           PPPOE_BASE_KEY "/use_tray"

/* Object/Class definition */
typedef struct _PppoeDialer PppoeDialer;
typedef struct _PppoeDialerClass PppoeDialerClass;

/* Object structure */
struct _PppoeDialerClass {
	PppPtyDialerClass parent_class;
};

struct _PppoeDialer {
	PppPtyDialer parent;
};

GType pppoe_dialer_get_type ();
PppDialer* pppoe_dialer_new (PppManager *manager);

G_END_DECLS

#endif /* __PPPOE_DIALER_H__ */
