#include "pppoe_dialer.h"

/* gtk/glade */
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

/* gnome */
#include <gconf/gconf-client.h>
#include <libgnomeui/libgnomeui.h>

/* std */
#include <string.h>

/* private */
#include <prefix.h>
#include <config.h>
#include <tlib/tlib.h>
#include <pppd.h>
#include "network.h"

typedef struct _PppoeDialerPrivate PppoeDialerPrivate;

/* ppp dialer */
static void pppoe_dialer_save_changes (PppDialer *self);
static gboolean pppoe_dialer_connect_is_sensitive (PppDialer *self);
static void pppoe_dialer_gui_init (PppDialer *self);
static gboolean pppoe_dialer_can_connect (PppDialer *self);

/* pty dialer */
static gchar * pppoe_dialer_gen_pty (PppPtyDialer *self);

/* pppoe dialer */
static gboolean pppoe_dialer_interface (GtkEntry *interface, GdkEventFocus *event, PppoeDialerPrivate *p);
static void pppoe_dialer_on_conf_update (GConfClient *c, guint conn_id, GConfEntry *e, PppoeDialerPrivate *p);
static void pppoe_dialer_on_about (PppManager *manager, GtkWidget *origin, gpointer data);

#define PPPOE_DIALER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PPPOE_TYPE_DIALER, PppoeDialerPrivate))

struct _PppoeDialerPrivate {
	GtkEntry *interface;
};

/******** gobject **********/

G_DEFINE_TYPE (PppoeDialer, pppoe_dialer, PPP_TYPE_PTY_DIALER);

static void
pppoe_dialer_class_init(PppoeDialerClass * klass)
{
	GObjectClass      *g_klass;
	PppDialerClass    *p_klass;
	PppPtyDialerClass *t_klass;
	
	g_klass = G_OBJECT_CLASS (klass);
	p_klass = PPP_DIALER_CLASS (klass);
	t_klass = PPP_PTY_DIALER_CLASS (klass);
	
	g_type_class_add_private(g_klass, sizeof(PppoeDialerPrivate));
	
	p_klass->save_changes = pppoe_dialer_save_changes;
	p_klass->connect_is_sensitive = pppoe_dialer_connect_is_sensitive;
	p_klass->gui_init = pppoe_dialer_gui_init;
	p_klass->can_connect = pppoe_dialer_can_connect;
	
	t_klass->gen_pty = pppoe_dialer_gen_pty;
}

PppDialer *
pppoe_dialer_new(PppManager *manager)
{
	PppDialer *d;
	d = PPP_DIALER (g_object_new (
		PPPOE_TYPE_DIALER,
		/* GtkWindow */
		"type", GTK_WINDOW_TOPLEVEL,
		/* PppDialer */
		"manager", manager,
		/* PtyDialer */
		"application", "pppoe",
		"account", "pppoe",
		NULL
	));
	return d;
}

static void
pppoe_dialer_init(PppoeDialer * object)
{}

/********* ppp dialer ******/

static void
pppoe_dialer_gui_init (PppDialer *self)
{
	GtkTable *table;
	GtkWidget *w;
	GList *ifaces, *iter;
	PppoeDialerPrivate *p;
	GConfClientNotifyFunc f;
	gchar *str;
	PppManager *manager = NULL;
	GConfClient *gconf;
	
	/* chain up */
	PPP_DIALER_CLASS (pppoe_dialer_parent_class)->gui_init (self);
	
	p = PPPOE_DIALER_GET_PRIVATE(self);
	manager = ppp_dialer_get_manager (self);
	
	/* listen to about */
	g_signal_connect(manager, "about", G_CALLBACK(pppoe_dialer_on_about), p);
	
	/* setup GUI */
	ppp_dialer_set_title(PPP_DIALER (self), _("PPPoE Dialer"));
	
	/* hide preferences button */
	gtk_widget_hide(ppp_dialer_get_preferences_button(PPP_DIALER (self)));

	table = ppp_dialer_get_table(PPP_DIALER (self));

	w = gtk_label_new(_("Interface:"));
	gtk_misc_set_alignment(GTK_MISC(w), 0, 0.5);
	gtk_widget_show(w);
	gtk_table_attach_defaults (table, w, 0, 1, 4, 5);

	w = gtk_combo_box_entry_new_text();
	gtk_widget_show(w);
	gtk_table_attach_defaults (table, w, 1, 2, 4, 5);
	p->interface = GTK_ENTRY (GTK_BIN (w)->child);
	
	g_signal_connect(p->interface, "focus-out-event", G_CALLBACK(pppoe_dialer_interface), p);
	
	ifaces = list_ethernet_ifaces();
	for (iter = g_list_first(ifaces); iter; iter = iter->next) {
		/* it's cheaper to just cast */
		gtk_combo_box_append_text((GtkComboBox *)w, (const gchar *) iter->data);
		g_free(iter->data);
	}
	g_list_free(ifaces);
	
	/* listen for configuration changes */
	gconf = gconf_client_get_default ();
	gconf_client_add_dir (gconf, PPPOE_BASE_KEY, GCONF_CLIENT_PRELOAD_NONE, NULL);
	f = (GConfClientNotifyFunc) pppoe_dialer_on_conf_update;
	gconf_client_notify_add (gconf, PPPOE_KEY_INTERFACE, f, p, NULL, NULL);
	
	/* get the default values */
	str = gconf_client_get_string(gconf, PPPOE_KEY_INTERFACE, NULL);
	if (str) {
		gtk_entry_set_text(p->interface, str);
		g_free (str);
	}
	
	g_object_unref (gconf);
}

static gboolean
pppoe_dialer_connect_is_sensitive (PppDialer *self)
{
	g_assert (PPPOE_IS_DIALER (self));
	if (!PPPOE_DIALER_GET_PRIVATE(self)->interface)
		return TRUE;
	return strlen(gtk_entry_get_text(PPPOE_DIALER_GET_PRIVATE(self)->interface)) > 0;
}

static gboolean
pppoe_dialer_can_connect (PppDialer *self)
{
	GList *ifaces, *iter;
	const gchar *iface = gtk_entry_get_text (PPPOE_DIALER_GET_PRIVATE (self)->interface);
	gboolean can_connect = FALSE;
	
	ifaces = list_ethernet_ifaces();
	for (iter = g_list_first(ifaces); !can_connect && iter; iter = iter->next) {
		/* it's cheaper to just cast */
		can_connect = !strcmp (iter->data, iface);
		g_free(iter->data);
	}
	g_list_free(ifaces);
	
	if (!can_connect) {
		gchar *msg;
		msg = g_strdup_printf (_("Cannot find interface <i>%s</i>"), iface);
		t_error (GTK_WINDOW (self), msg, _("The interface might be plugged off or not correctly configured."));
		g_free (msg);
	}
	
	return can_connect && PPP_DIALER_CLASS (pppoe_dialer_parent_class)->can_connect (self);
}

static void
pppoe_dialer_save_changes (PppDialer *self)
{
	const gchar *text = NULL;
	PppoeDialerPrivate *p = PPPOE_DIALER_GET_PRIVATE (self);
	GConfClient *gconf;
	
	gconf = gconf_client_get_default ();
	text = gtk_entry_get_text(p->interface);
	gconf_client_set_string(gconf, PPPOE_KEY_INTERFACE, text, NULL);
	g_object_unref (gconf);
}

/******* pty dialer ********/

static gchar *
pppoe_dialer_gen_pty (PppPtyDialer *self)
{
	gchar *pty;
	const gchar *eth;
	gchar *pppoe;
	PppoeDialerPrivate *p = PPPOE_DIALER_GET_PRIVATE (self);	
	eth = gtk_entry_get_text (p->interface);
	pppoe = ppp_pty_dialer_get_application_path (self);
	
	if (eth && pppoe) {
		pty = g_strdup_printf("%s -I %s -m 1412 -T 80 -U", pppoe, eth);
	} else {
		/* return a bogus pty that will always return an error */
		pty = g_strup ("/bin/false");
	}
	if (pppoe)
		g_free (pppoe);
	return pty;
}

/******* pppoe dialer ******/

static gboolean
pppoe_dialer_interface (GtkEntry *interface, GdkEventFocus *event, PppoeDialerPrivate *p)
{
	const gchar *text = NULL;
	GConfClient *gconf;
	
	text = gtk_entry_get_text(interface);
	gconf = gconf_client_get_default ();
	gconf_client_set_string(gconf, PPPOE_KEY_INTERFACE, text, NULL);
	g_object_unref (gconf);
	return FALSE;
}

static void
pppoe_dialer_on_conf_update (GConfClient *c, guint conn_id, GConfEntry *e, PppoeDialerPrivate *p)
{
	if (!strcmp(e->key, PPPOE_KEY_INTERFACE)) {
		gtk_entry_set_text(p->interface, gconf_value_get_string(e->value));
	}
}

static void
pppoe_dialer_set_origin_sensitive (GtkWidget *dlg, gint response, GtkWidget *origin)
{
	gtk_widget_set_sensitive (origin, TRUE);
}

static void
pppoe_dialer_on_about (PppManager *manager, GtkWidget *origin, gpointer data)
{
	GtkWidget *about_dlg = NULL;
	const char *authors[] = {
		"Tiago Cogumbreiro <cogumbreiro@sf.users.net>",
		"Milton Moura <mgcm@acm.org>",
		NULL
	};
	const char *translators[] = {
		/* translators: change this string with your name
		 */
		_("No translators assigned"),
		NULL
	};
	
	about_dlg = GTK_WIDGET(gnome_about_new(
		"GNOME PPX",
		VERSION,
		"Copyright (C) 2004 Tiago Cogumbreiro\nCopyright (C) 2004 Milton Moura",
		_("PPPoE Dialer"),
		authors,
		translators,
		NULL,
		NULL));
	g_return_if_fail(about_dlg);
	/* make menu item insensitive until user did not close about dialog */
	g_signal_connect (about_dlg, "response", G_CALLBACK (pppoe_dialer_set_origin_sensitive), origin);
	gtk_widget_set_sensitive (origin, FALSE);
	gtk_widget_show_all (about_dlg);
}
