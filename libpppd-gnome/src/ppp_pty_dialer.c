#include "ppp_pty_dialer.h"
#include "ppp_state_listener.h"

#include <string.h>
#include <pppd.h>
#include <glib/gi18n.h>
#include <config.h>
#include <tlib/tlib.h>

typedef struct _PppPtyDialerPrivate PppPtyDialerPrivate;

static void ppp_pty_dialer_finalize(GObject * object);
static void ppp_pty_dialer_set_property(GObject * object, guint prop_id,
				    const GValue * value,
				    GParamSpec * pspec);

static gboolean ppp_pty_dialer_on_startup (PppStateListener *self, GObject *origin);

static void ppp_pty_dialer_listener_init (PppStateListenerIface *iface);

static gboolean ppp_pty_dialer_can_connect (PppDialer *self);
static GSList* ppp_pty_dialer_get_options (PppDialer *self);
static void ppp_pty_dialer_gui_init (PppDialer *self);

#define PPP_PTY_DIALER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PPP_TYPE_PTY_DIALER, PppPtyDialerPrivate))

static gpointer ppp_pty_dialer_parent_iface = NULL;

enum {
	PROP_0,
	PROP_APPLICATION,
};

struct _PppPtyDialerPrivate {
	/* application's name */
	gchar *app_name;
	gchar *app_key;
	GSList *options;
	pppd_option *opt_pty;
};

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (PppPtyDialer, ppp_pty_dialer, PPP_TYPE_DIALER,
	G_IMPLEMENT_INTERFACE (
		PPP_TYPE_STATE_LISTENER,
		ppp_pty_dialer_listener_init
	)
);

static void ppp_pty_dialer_class_init(PppPtyDialerClass * klass)
{
	GObjectClass   *g_klass;
	PppDialerClass *p_klass;
	
	g_klass = G_OBJECT_CLASS (klass);
	p_klass = PPP_DIALER_CLASS (klass);
	
	g_type_class_add_private(g_klass, sizeof(PppPtyDialerPrivate));
	
	g_klass->finalize = ppp_pty_dialer_finalize;

	p_klass->get_options = ppp_pty_dialer_get_options;
	p_klass->can_connect = ppp_pty_dialer_can_connect;
	p_klass->gui_init    = ppp_pty_dialer_gui_init;

	g_klass->set_property = ppp_pty_dialer_set_property;
	g_object_class_install_property (g_klass, PROP_APPLICATION,
					g_param_spec_string("application",
							    "The pty aplication",
							    "Application which pty will call",
							    NULL,
							    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void ppp_pty_dialer_init(PppPtyDialer * object)
{
	PppPtyDialerPrivate *p;
	p = PPP_PTY_DIALER_GET_PRIVATE(object);
	p->options = NULL;
	
	/* now it's pty */
	p->opt_pty = pppd_option_new("pty");
	p->options = g_slist_append (p->options, p->opt_pty);
}

static void ppp_pty_dialer_finalize(GObject * object)
{
	PppPtyDialerPrivate *p = PPP_PTY_DIALER_GET_PRIVATE(object);
	/* chain up */
	G_OBJECT_CLASS (ppp_pty_dialer_parent_class)->finalize (object);
	/* free options refs */
	g_slist_foreach (p->options, (GFunc) pppd_option_free, NULL);
	g_slist_free (p->options);
	g_free (p->app_key);
	g_free (p->app_name);
}

static void ppp_pty_dialer_listener_init (PppStateListenerIface *iface)
{
	ppp_pty_dialer_parent_iface = g_type_interface_peek_parent (iface);
	iface->on_startup = ppp_pty_dialer_on_startup;
}

static void ppp_pty_dialer_set_property(GObject * object, guint prop_id,
				    const GValue * value,
				    GParamSpec * pspec)
{
	PppPtyDialerPrivate *p = PPP_PTY_DIALER_GET_PRIVATE (object);

	switch (prop_id) {
		case PROP_APPLICATION:
			p->app_name = g_strdup (g_value_get_string (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

/************** end of boiler plate ***************/

static GSList* ppp_pty_dialer_get_options (PppDialer *self)
{
	return PPP_PTY_DIALER_GET_PRIVATE(self)->options;
}

static gboolean ppp_pty_dialer_can_connect (PppDialer *self)
{
	/* finally lets update the pty entry */
	pppd_option_set_string (
		PPP_PTY_DIALER_GET_PRIVATE(self)->opt_pty,
		PPP_PTY_DIALER_GET_CLASS (self)->gen_pty (PPP_PTY_DIALER (self))
	);
	return TRUE;
}

static void ppp_pty_dialer_gui_init (PppDialer *self)
{
	PppPtyDialerPrivate *p;
	p = PPP_PTY_DIALER_GET_PRIVATE (self);
	/* generate application's gconf key, unfortunately gobject does not provide
	 * a mechanism to extend properties.
	 */
	g_return_if_fail (p->app_name);
	p->app_key = g_strconcat (ppp_manager_get_gconf_dir (ppp_dialer_get_manager (self)), "/", p->app_name, "_path", NULL);
}

static gboolean ppp_pty_dialer_on_startup (PppStateListener *self, GObject *origin)
{
	gboolean ret;
	PppPtyDialerPrivate *p = PPP_PTY_DIALER_GET_PRIVATE (self);
	
	g_assert (p->app_name);
	g_assert (p->app_key);
	
	ret = PPP_STATE_LISTENER_CLASS (ppp_pty_dialer_parent_iface)->on_startup (self, origin);
	/* the pty app must have suid too */
	ret = ret && ppp_dialer_check_app (p->app_key, p->app_name);
	return ret && t_update_program_in_gconf_gui (GTK_WINDOW (self), p->app_name, p->app_key);
}

gchar *ppp_pty_dialer_get_application_path (PppPtyDialer *self)
{
	GConfClient *gconf;
	gchar *path;
	GError *err = NULL;
	
	g_assert (PPP_PTY_DIALER_GET_PRIVATE (self)->app_key);
	gconf = gconf_client_get_default ();
	path = gconf_client_get_string (gconf, PPP_PTY_DIALER_GET_PRIVATE (self)->app_key, &err);
	T_ERR_WARN (err);
	/* we don't need it anymore */
	g_object_unref (gconf);
	return path;
}
