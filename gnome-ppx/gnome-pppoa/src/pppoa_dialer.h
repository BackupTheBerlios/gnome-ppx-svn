#ifndef __PPPOA_DIALER_H__
#define __PPPOA_DIALER_H__

#include <glib.h>
#include <glib-object.h>
#include <pppd_gnome.h>

G_BEGIN_DECLS

#define PPPOA_TYPE_DIALER            (pppoa_dialer_get_type())
#define PPPOA_DIALER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PPPOA_TYPE_DIALER, PppoaDialer))
#define PPPOA_DIALER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PPPOA_TYPE_DIALER, PppoaDialerClass))
#define PPPOA_IS_DIALER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPPOA_TYPE_DIALER))
#define PPPOA_IS_DIALER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),PPPOA_TYPE_DIALER))
#define PPPOA_DIALER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PPPOA_TYPE_DIALER, PppoaDialerClass))
#define PPPOA_BASE_KEY               "/apps/gnome-pppoa"
#define PPPOA_KEY_ADDRESS             PPPOA_BASE_KEY "/address"

/* Object/Class definition */
typedef struct _PppoaDialer PppoaDialer;
typedef struct _PppoaDialerClass PppoaDialerClass;

/* Object structure */
struct _PppoaDialerClass {
	PppDialerClass parent;
};

struct _PppoaDialer {
	PppDialer parent;
};

GType pppoa_dialer_get_type ();

PppDialer* pppoa_dialer_new (PppManager *manager, const gchar *addressbook);

G_END_DECLS
#endif /* __PPPOA_DIALER_H__ */
