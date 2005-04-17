#include "pptp_dialer.h"

#include <string.h>
#include <pppd_gnome.h>
#include <pppd.h>
#include <tlib/tlib.h>
#include <prefix.h>
#include <config.h>

/* gtk */
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glade/glade.h>

/* gnome */
#include <libgnomeui/libgnomeui.h>
#include <gconf/gconf-client.h>

#define PPTP_DIALER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PPTP_TYPE_DIALER, PptpDialerPrivate))

typedef struct _PptpDialerPrivate PptpDialerPrivate;

/* ppp dialer */
static void pptp_dialer_save_changes (PppDialer *self);
static gboolean pptp_dialer_connect_is_sensitive (PppDialer *self);
static void pptp_dialer_gui_init (PppDialer *self);

/* pty dialer */
static gchar * pptp_dialer_gen_pty (PppPtyDialer *self);

/* pptp dialer */
static gboolean pptp_dialer_server (GtkEntry *interface, GdkEventFocus *event, PptpDialerPrivate *p);
static void pptp_dialer_on_conf_update (GConfClient *c, guint conn_id, GConfEntry *e, PptpDialerPrivate *p);
static void pptp_dialer_on_about (PppManager *manager, GtkWidget *origin, gpointer data);
static void pptp_dialer_on_changed (GtkWidget *w, PptpDialerPrivate *p);

struct _PptpDialerPrivate {
	PptpDialer *self;
	GtkEntry *server;
};

G_DEFINE_TYPE (PptpDialer, pptp_dialer, PPP_TYPE_PTY_DIALER);

static void
pptp_dialer_class_init(PptpDialerClass * klass)
{
	GObjectClass *g_klass;
	PppDialerClass *p_klass;
	PppPtyDialerClass *t_klass;

	g_klass = G_OBJECT_CLASS (klass);
	p_klass = PPP_DIALER_CLASS (klass);
	t_klass = PPP_PTY_DIALER_CLASS (klass);
	
	g_type_class_add_private(g_klass, sizeof(PptpDialerPrivate));
	
	p_klass->save_changes = pptp_dialer_save_changes;
	p_klass->connect_is_sensitive = pptp_dialer_connect_is_sensitive;
	p_klass->gui_init = pptp_dialer_gui_init;
	
	t_klass->gen_pty = pptp_dialer_gen_pty;
}

PppDialer*
pptp_dialer_new (PppManager *manager)
{
	PppDialer *d;
	d = PPP_DIALER (g_object_new (
		PPTP_TYPE_DIALER,
		/* GtkWindow */
		"type", GTK_WINDOW_TOPLEVEL,
		/* PppDialer */
		"manager", manager,
		/* PtyDialer */
		"application", "pptp",
		"account", "pptp",
		NULL
	));
	return d;
}

static void
pptp_dialer_gui_init (PppDialer *self)
{
	PptpDialerPrivate *p;
	GtkTable *table;
	GConfClient *gconf;
	GtkWidget *w;
	GConfClientNotifyFunc f;
	gchar *str;

	/* chain up */
	PPP_DIALER_CLASS (pptp_dialer_parent_class)->gui_init (self);
	
	p = PPTP_DIALER_GET_PRIVATE(self);
		
	/* listen to about */
	g_signal_connect(ppp_dialer_get_manager (self), "about", G_CALLBACK(pptp_dialer_on_about), p);
	
	/* setup GUI */
	ppp_dialer_set_title (PPP_DIALER (self), _("PPTP Dialer"));
	
	/* hide preferences button */
	gtk_widget_hide (ppp_dialer_get_preferences_button (PPP_DIALER (self)));
	
	table = ppp_dialer_get_table(PPP_DIALER (self));

	w = gtk_label_new(_("Server:"));
	gtk_misc_set_alignment(GTK_MISC(w), 0, 0.5);
	gtk_widget_show(w);
	gtk_table_attach_defaults (table, w, 0, 1, 4, 5);

	w = gtk_entry_new();
	gtk_widget_show(w);
	gtk_table_attach_defaults (table, w, 1, 2, 4, 5);
	
	p->server = GTK_ENTRY(w);
	g_signal_connect(p->server, "changed", G_CALLBACK(pptp_dialer_on_changed), p);
	g_signal_connect(p->server, "focus-out-event", G_CALLBACK(pptp_dialer_server), p);
	
	/* listen for configuration changes */
	gconf = gconf_client_get_default();
	gconf_client_add_dir (gconf, PPTP_BASE_KEY, GCONF_CLIENT_PRELOAD_NONE, NULL);
	f = (GConfClientNotifyFunc) pptp_dialer_on_conf_update;
	gconf_client_notify_add (gconf, PPTP_KEY_SERVER, f, p, NULL, NULL);
	
	/* get the default values */
	str = gconf_client_get_string(gconf, PPTP_KEY_SERVER, NULL);
	if (str) {
		gtk_entry_set_text(p->server, str);
		g_free (str);
	}

	g_object_unref (gconf);
}

static void
pptp_dialer_init(PptpDialer * object)
{
	PPTP_DIALER_GET_PRIVATE (object)->self = object;
}

static gboolean
pptp_dialer_server (GtkEntry *server, GdkEventFocus *event, PptpDialerPrivate *p)
{
	const gchar *text = NULL;
	GError *err = NULL;
	GConfClient *gconf;
	
	text = gtk_entry_get_text(server);
	gconf = gconf_client_get_default ();
	gconf_client_set_string(gconf, PPTP_KEY_SERVER, text, &err);
	T_ERR_WARN (err);
	g_object_unref (gconf);
	return FALSE;
}

static void
pptp_dialer_on_conf_update (GConfClient *c, guint conn_id, GConfEntry *e, PptpDialerPrivate *p)
{
	if (!strcmp(e->key, PPTP_KEY_SERVER)) {
		gtk_entry_set_text(p->server, gconf_value_get_string(e->value));
	}
}

static gchar *
pptp_dialer_gen_pty (PppPtyDialer *self)
{
	gchar *pptp, *pty;
	const gchar *server;
	PptpDialerPrivate *p;
	
	p = PPTP_DIALER_GET_PRIVATE (self);
	pptp = ppp_pty_dialer_get_application_path (self);
	
	server = gtk_entry_get_text (p->server);
	
	if (pptp && server) {
		pty = g_strdup_printf ("%s %s --nolaunchpppd", pptp, server);
	} else {
		pty = g_strdup ("/bin/false");
	}
	if (pptp)
		g_free (pptp);
	return pty;
}

static void
pptp_dialer_set_origin_sensitive (GtkWidget *dlg, gint response, GtkWidget *origin)
{
	gtk_widget_set_sensitive (origin, TRUE);
}

static void
pptp_dialer_on_about (PppManager *manager, GtkWidget *origin, gpointer data)
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
		_("PPTP Dialer"),
		authors,
		translators,
		NULL,
		NULL));
	g_return_if_fail(about_dlg);
	/* make menu item insensitive until user did not close about dialog */
	g_signal_connect (about_dlg, "response", G_CALLBACK (pptp_dialer_set_origin_sensitive), origin);
	gtk_widget_set_sensitive (origin, FALSE);
	gtk_widget_show_all (about_dlg);
}

static gboolean
pptp_dialer_connect_is_sensitive (PppDialer *self)
{
	if (!PPTP_DIALER_GET_PRIVATE (self)->server)
		return FALSE;
	
	return strlen(gtk_entry_get_text (PPTP_DIALER_GET_PRIVATE (self)->server)) > 0;
}

static void
pptp_dialer_on_changed (GtkWidget *w, PptpDialerPrivate *p)
{
	/* something changed, let's update the connect button sensitivity */
	ppp_dialer_update_connect_sensitivity (PPP_DIALER (p->self));
}

static void
pptp_dialer_save_changes (PppDialer *self)
{
	const gchar *text = NULL;
	GConfClient *gconf;
	GError *err = NULL;
	PptpDialerPrivate *p;
	p = PPTP_DIALER_GET_PRIVATE (self);
	gconf = gconf_client_get_default ();
	text = gtk_entry_get_text(p->server);
	gconf_client_set_string(gconf, PPTP_KEY_SERVER, text, &err);
	T_ERR_WARN (err);
	g_object_unref (gconf);
}
