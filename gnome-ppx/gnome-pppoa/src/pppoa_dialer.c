#include "pppoa_dialer.h"
#include "pppoa_db.h"
#include "pppoa_db_gui.h"

/* gnome */
#include <gconf/gconf-client.h>
#include <libgnomeui/libgnomeui.h>

/* gtk + glade */
#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

/* std */
#include <string.h>

/* priv */
#include <pppd_gnome.h>
#include <pppd.h>
#include <config.h>
#include <tlib/tlib.h>

#define PPPOA_DIALER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PPPOA_TYPE_DIALER, PppoaDialerPrivate))
typedef struct _PppoaDialerPrivate PppoaDialerPrivate;

/* gobject */
static void pppoa_dialer_finalize (GObject * object);
static void pppoa_dialer_set_property (GObject * object, guint prop_id,
				    const GValue * value,
				    GParamSpec * pspec);

/* ppp dialer */
static void pppoa_dialer_gui_init (PppDialer *self);
static GSList *pppoa_dialer_get_options (PppDialer *self);
static gboolean pppoa_dialer_can_connect (PppDialer *self);
static gboolean pppoa_dialer_connect_is_sensitive (PppDialer *self);

/* pppoa dialer */
static void pppoa_dialer_on_conf_update (GConfClient *c, guint conn_id, GConfEntry *e, PppoaDialerPrivate *p);
static void pppoa_dialer_on_about (PppManager *manager, GtkWidget *origin, gpointer data);
static void pppoa_dialer_set_addressbook (PppoaDialer *self, const gchar *addressbook);
static void pppoa_dialer_on_db_gui_changed (PppoaDbGui *db_gui, PppoaDialer *self);

struct _PppoaDialerPrivate {
	PppoaDbGui *db;
	GConfClient *gconf;
	pppd_option *opt_vpi_vci;
	GSList *options;
};

enum {
	PROP_0,
	PROP_ADDRESSBOOK,
};

/* gobject */

G_DEFINE_TYPE (PppoaDialer, pppoa_dialer, PPP_TYPE_DIALER);


static void
pppoa_dialer_class_init (PppoaDialerClass * klass)
{
	GObjectClass *g_klass;
	PppDialerClass *p_klass;

	g_klass = G_OBJECT_CLASS (klass);
	p_klass = PPP_DIALER_CLASS (klass);
	
	g_type_class_add_private(g_klass, sizeof(PppoaDialerPrivate));
	g_klass->finalize = pppoa_dialer_finalize;
	g_klass->set_property = pppoa_dialer_set_property;
	
	p_klass->gui_init = pppoa_dialer_gui_init;
	p_klass->get_options = pppoa_dialer_get_options;
	p_klass->can_connect = pppoa_dialer_can_connect;
	p_klass->connect_is_sensitive = pppoa_dialer_connect_is_sensitive;
	
	g_object_class_install_property (g_klass, PROP_ADDRESSBOOK,
					g_param_spec_string("addressbook",
							    "The database filename",
							    "The database filename",
							    NULL,
							    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

}

PppDialer *
pppoa_dialer_new(PppManager *manager, const gchar *addressbook)
{
	return PPP_DIALER (g_object_new (
		PPPOA_TYPE_DIALER,
		/* gtk window */
		"type", GTK_WINDOW_TOPLEVEL,
		/* ppp dialer */
		"manager", manager,
		/* pppoa dialer */
		"addressbook", addressbook,
		"account", "pppoa",
		NULL));
}

static void
pppoa_dialer_init(PppoaDialer * object)
{
	PppoaDialerPrivate *p;
	pppd_option *opt;
	p = PPPOA_DIALER_GET_PRIVATE(object);
	p->options = NULL;

	/* now it's plugin */
	opt = pppd_option_new("plugin");
	pppd_option_set_const_string (opt, "pppoatm.so");
	p->options = g_slist_append (p->options, opt);
	
	/* finally vpi and vci */
	p->opt_vpi_vci = pppd_option_new ("");
	p->options = g_slist_append (p->options, p->opt_vpi_vci);
}

static void
pppoa_dialer_finalize(GObject * object)
{
	PppoaDialerPrivate *p = PPPOA_DIALER_GET_PRIVATE(object);
	g_slist_foreach (p->options, (GFunc) pppd_option_free, NULL);
	g_slist_free (p->options);
	g_object_unref (p->db);
	/* chain up finalize */
	G_OBJECT_CLASS (pppoa_dialer_parent_class)->finalize (object);
}

static void
pppoa_dialer_set_property (GObject * object, guint prop_id,
				    const GValue * value,
				    GParamSpec * pspec)
{
	if (prop_id == PROP_ADDRESSBOOK)
		pppoa_dialer_set_addressbook (PPPOA_DIALER (object), g_value_get_string (value));
	else
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
}

/* ppp dialer */

static void
pppoa_dialer_update_db_gui (PppoaDialerPrivate *p, gchar *str)
{
	gchar *code;
	gchar *isp;
	
	/* split the string in 2 */
	str[2] = '\0';
	
	code = str;
	isp = str + 3;
	
	pppoa_db_gui_activate (p->db, code, isp);
}

static void
pppoa_dialer_gui_init (PppDialer *self)
{
	PppoaDialerPrivate *p;
	GtkWidget *w;
	GConfClientNotifyFunc f;
	gchar *str;
	GtkTable *table;
	GError *error = NULL;
	
	p = PPPOA_DIALER_GET_PRIVATE (self);
	
	ppp_dialer_set_title (self, _("PPPoA Dialer"));
	
	gtk_widget_hide (ppp_dialer_get_preferences_button (self));
	
	table = ppp_dialer_get_table (self);
	p->gconf = gconf_client_get_default();

	/* Country */
	w = gtk_label_new(_("Country:"));
	gtk_misc_set_alignment(GTK_MISC(w), 0, 0.5);
	gtk_widget_show(w);
	gtk_table_attach_defaults (table, w, 0, 1, 4, 5);

	gtk_table_attach_defaults (table, GTK_WIDGET (p->db->countries), 1, 2, 4, 5);
	gtk_widget_show (GTK_WIDGET (p->db->countries));
	
	/* ISP */
	w = gtk_label_new (_("ISP:"));
	gtk_misc_set_alignment(GTK_MISC(w), 0, 0.5);
	gtk_widget_show(w);
	gtk_table_attach_defaults (table, w, 0, 1, 5, 6);
	
	gtk_table_attach_defaults (table, GTK_WIDGET (p->db->isps), 1, 2, 5, 6);
	gtk_widget_show (GTK_WIDGET (p->db->isps));
	
	/* listen for configuration changes */
	gconf_client_add_dir (p->gconf, PPPOA_BASE_KEY, GCONF_CLIENT_PRELOAD_NONE, NULL);
	f = (GConfClientNotifyFunc) pppoa_dialer_on_conf_update;
	gconf_client_notify_add (p->gconf, PPPOA_KEY_ADDRESS, f, p, NULL, NULL);
	
	/* get the default values */
	str = gconf_client_get_string(p->gconf, PPPOA_KEY_ADDRESS, &error);
	T_ERR_WARN (error);
	if (str) {
		/* now we have this in format: country;isp */
		if (strlen (str) > 4) {
			pppoa_dialer_update_db_gui (p, str);
		}
		g_free (str);
	}

	/* generate the event */
	g_signal_connect (ppp_dialer_get_manager (self), "about", G_CALLBACK(pppoa_dialer_on_about), p);
	g_signal_connect (p->db, "changed", G_CALLBACK (pppoa_dialer_on_db_gui_changed), self);
}

/* pppoa dialer */
static void
pppoa_dialer_set_addressbook (PppoaDialer *self, const gchar *addressbook)
{
	PppoaDialerPrivate *p;
	PppoaDb *db;
	
	p = PPPOA_DIALER_GET_PRIVATE (self);
	db = pppoa_db_new (addressbook);
	p->db = pppoa_db_gui_new (db);
	g_object_unref (db);
}

static void
pppoa_dialer_on_conf_update (GConfClient *c, guint conn_id, GConfEntry *e, PppoaDialerPrivate *p)
{
	if (!strcmp(e->key, PPPOA_KEY_ADDRESS)) {
		gchar *val;
		val = g_strdup (gconf_value_get_string (e->value));
		if (val) {
			pppoa_dialer_update_db_gui (p, val);
			g_free (val);
		}
	}
}

static GSList*
pppoa_dialer_get_options (PppDialer *self)
{
	return PPPOA_DIALER_GET_PRIVATE(self)->options;
}

static gboolean
pppoa_dialer_can_connect (PppDialer *self)
{
	gchar *pppoa, *vpi_vci;
	gint vpi, vci;
	PppoaDialerPrivate *p;

	/* check preconditions */
	/* TODO: this must be correct */
	p = PPPOA_DIALER_GET_PRIVATE(self);
	
	vpi = pppoa_db_gui_get_selected_vpi (p->db);
	vci = pppoa_db_gui_get_selected_vci (p->db);

	g_return_val_if_fail(pppoa, FALSE);
	/* finally lets update the pty entry */
	vpi_vci = g_strdup_printf ("%d.%d", vpi, vci);
	pppd_option_set_void (p->opt_vpi_vci, vpi_vci);
	g_free (vpi_vci);
	
	return TRUE;
}

static void
pppoa_dialer_set_origin_sensitive (GtkWidget *dlg, gint response, GtkWidget *origin)
{
	gtk_widget_set_sensitive (origin, TRUE);
}

static void
pppoa_dialer_on_about (PppManager *manager, GtkWidget *origin, gpointer data)
{
	GtkWidget *about_dlg = NULL;
	const char *authors[] = {
		"Tiago Cogumbreiro <cogumbreiro@sf.users.net>",
		"Milton Moura <mgcm@acm.org>",
		NULL
	};
	const char *translators[] = {NULL};
	about_dlg = GTK_WIDGET(gnome_about_new(
		"GNOME PPX",
		VERSION,
		"Copyright (C) 2004 Tiago Cogumbreiro\nCopyright (C) 2004 Milton Moura",
		_("PPPoA Dialer"),
		authors,
		translators,
		NULL,
		NULL));
	g_return_if_fail(about_dlg);
	/* make menu item insensitive until user did not close about dialog */
	g_signal_connect (about_dlg, "response", G_CALLBACK (pppoa_dialer_set_origin_sensitive), origin);
	gtk_widget_set_sensitive (origin, FALSE);
	gtk_widget_show_all (about_dlg);
}

static void
pppoa_dialer_on_db_gui_changed (PppoaDbGui *db_gui, PppoaDialer *self)
{
	GError *err = NULL;
	gchar *address;
	const gchar *code, *isp;
	
	code = pppoa_db_gui_get_selected_code (db_gui);
	isp = pppoa_db_gui_get_selected_isp (db_gui);
	g_return_if_fail (code || isp);
	
	address = g_strconcat (code, "/", isp, NULL);
	gconf_client_set_string (PPPOA_DIALER_GET_PRIVATE (self)->gconf, PPPOA_KEY_ADDRESS, address, &err);
	T_ERR_WARN (err);
	g_free (address);
	
	ppp_dialer_update_connect_sensitivity (PPP_DIALER (self));
}

static gboolean
pppoa_dialer_connect_is_sensitive (PppDialer *self)
{
	PppoaDbGui *db = PPPOA_DIALER_GET_PRIVATE (self)->db;
	
	return db && pppoa_db_gui_get_selected_code (db) && pppoa_db_gui_get_selected_isp (db);
}
