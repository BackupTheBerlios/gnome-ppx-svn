#include "ppp_state_listener.h"
#include <tlib/tlib.h>

G_DEFINE_INTERFACE_TYPE (PppStateListener, ppp_state_listener);

static void ppp_state_listener_init (PppStateListenerIface *iface) {}

void ppp_state_listener_on_idle(PppStateListener * self, GObject * origin)
{
	PPP_STATE_LISTENER_GET_IFACE(self)->on_idle(self, origin);
}

void ppp_state_listener_on_connecting(PppStateListener * self,
				      GObject * origin)
{
	PPP_STATE_LISTENER_GET_IFACE(self)->on_connecting(self, origin);
}

gboolean ppp_state_listener_on_startup(PppStateListener * self,
				   GObject * origin)
{
	return PPP_STATE_LISTENER_GET_IFACE(self)->on_startup(self, origin);
}

void ppp_state_listener_on_connected(PppStateListener * self,
				     GObject * origin,
				     gboolean new_connection)
{
	PPP_STATE_LISTENER_GET_IFACE(self)->on_connected(self, origin, new_connection);
}

void ppp_state_listener_on_tray_usage(PppStateListener * self,
				      GObject * origin, gboolean use_tray)
{
	PPP_STATE_LISTENER_GET_IFACE(self)->on_tray_usage(self, origin, use_tray);
}
