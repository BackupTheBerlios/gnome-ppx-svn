
#ifndef __PPP_STATE_LISTENER_H__
#define __PPP_STATE_LISTENER_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PPP_TYPE_STATE_LISTENER            (ppp_state_listener_get_type())
#define PPP_STATE_LISTENER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PPP_TYPE_STATE_LISTENER, PppStateListener))
#define PPP_STATE_LISTENER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PPP_TYPE_STATE_LISTENER, PppStateListenerIface))
#define PPP_IS_STATE_LISTENER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPP_TYPE_STATE_LISTENER))
#define PPP_IS_STATE_LISTENER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),PPP_TYPE_STATE_LISTENER))
#define PPP_STATE_LISTENER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PPP_TYPE_STATE_LISTENER, PppStateListenerIface))

/* Object/Class definition */
typedef struct _PppStateListener PppStateListener;
typedef struct _PppStateListenerIface PppStateListenerIface;

typedef void     (*PSLOnIdleFunc) (PppStateListener* self, GObject* origin);
typedef void     (*PSLOnConnectingFunc) (PppStateListener* self, GObject* origin);
typedef void     (*PSLOnConnectedFunc) (PppStateListener* self, GObject* origin, gboolean new_connection);
typedef void     (*PSLOnTrayUsageFunc) (PppStateListener* self, GObject* origin, gboolean use_tray);
typedef gboolean (*PSLOnStartupFunc) (PppStateListener* self, GObject* origin);

/* Object structure */
struct _PppStateListenerIface {
	GTypeInterface parent_iface;
	PSLOnIdleFunc       on_idle;
	PSLOnConnectingFunc on_connecting;
	PSLOnStartupFunc    on_startup;
	PSLOnConnectedFunc  on_connected;
	PSLOnTrayUsageFunc  on_tray_usage;
};

GType ppp_state_listener_get_type ();

void ppp_state_listener_on_idle (PppStateListener* self, GObject* origin);
void ppp_state_listener_on_connecting (PppStateListener* self, GObject* origin);
gboolean ppp_state_listener_on_startup (PppStateListener* self, GObject* origin);
void ppp_state_listener_on_connected (PppStateListener* self, GObject* origin, gboolean new_connection);
void ppp_state_listener_on_tray_usage (PppStateListener* self, GObject* origin, gboolean use_tray);

G_END_DECLS

#endif /* __PPP_STATE_LISTENER_H__ */
