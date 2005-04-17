#ifndef __PPTP_DIALER_H__
#define __PPTP_DIALER_H__
#include <glib.h>
#include <glib-object.h>
#include <pppd_gnome.h>

G_BEGIN_DECLS

#define PPTP_TYPE_DIALER            (pptp_dialer_get_type())
#define PPTP_DIALER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PPTP_TYPE_DIALER, PptpDialer))
#define PPTP_DIALER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PPTP_TYPE_DIALER, PptpDialerClass))
#define PPTP_IS_DIALER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPTP_TYPE_DIALER))
#define PPTP_IS_DIALER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),PPTP_TYPE_DIALER))
#define PPTP_DIALER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PPTP_TYPE_DIALER, PptpDialerClass))
#define PPTP_BASE_KEY               "/apps/gnome-pptp"
#define PPTP_KEY_SERVER             PPTP_BASE_KEY "/server"
#define PPTP_KEY_PPTP_PATH          PPTP_BASE_KEY "/pptp_path"
#define PPTP_KEY_USE_TRAY           PPTP_BASE_KEY "/use_tray"

/* Object/Class definition */
typedef struct _PptpDialer PptpDialer;
typedef struct _PptpDialerClass PptpDialerClass;

/* Object structure */
struct _PptpDialerClass {
	PppPtyDialerClass parent;
};

/* Class structure */
struct _PptpDialer {
	PppPtyDialer parent;
};

GType pptp_dialer_get_type ();
PppDialer* pptp_dialer_new (PppManager *manager);

G_END_DECLS
#endif /* __PPTP_DIALER_H__ */
