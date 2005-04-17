#include "ppp_progress.h"
#include "ppp_manager.h"
#include "ppp_state_listener.h"
#include <pppd.h>
#include <glib/gi18n.h>
#include <tlib/tlib.h>

#define PPP_PROGRESS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PPP_TYPE_PROGRESS, PppProgressPrivate))

typedef struct _PppProgressPrivate PppProgressPrivate;

static void ppp_progress_finalize(GObject * object);
static void ppp_progress_set_property(GObject * object, guint prop_id,
				  const GValue * value,
				  GParamSpec * pspec);

static void ppp_progress_destroy_dialog(PppStateListener * self);

static void ppp_progress_on_connecting(PppStateListener * self,
				       GObject * origin);
static void ppp_progress_on_connected(PppStateListener * self,
				      GObject * origin,
				      gboolean new_connection);
static void ppp_progress_on_idle (PppStateListener* self, GObject* origin);
static void ppp_progress_state_listener_init(PppStateListenerIface *
					     iface);
static void ppp_progress_cancel (GtkDialog *dialog, gint response, PppProgressPrivate *p);
/****** end of prototypes definitions **********/

struct _PppProgressPrivate {
	PppManager *manager;
	GtkWidget *dlg_connecting;
};

enum {
	PROP_0,
	PROP_MANAGER,
};

G_DEFINE_TYPE_WITH_CODE (PppProgress, ppp_progress, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE (
		PPP_TYPE_STATE_LISTENER,
		ppp_progress_state_listener_init
	)
);

static void
ppp_progress_class_init(PppProgressClass * klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(gobject_class, sizeof(PppProgressPrivate));
	gobject_class->finalize = ppp_progress_finalize;
	gobject_class->set_property = ppp_progress_set_property;

	g_object_class_install_property (gobject_class, PROP_MANAGER,
				g_param_spec_object("manager",
							"manager",
							"manager",
							PPP_TYPE_MANAGER,
							G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

}

PppProgress *
ppp_progress_new(PppManager *manager)
{
	return PPP_PROGRESS(g_object_new(PPP_TYPE_PROGRESS, "manager", manager, NULL));
}

static void 
ppp_progress_init(PppProgress * object)
{
	PppProgressPrivate *private_data;

	private_data = PPP_PROGRESS_GET_PRIVATE(object);
	private_data->dlg_connecting = NULL;
}

static void
ppp_progress_finalize(GObject * object)
{
	/* make sure dialog is destroyed */
	ppp_progress_destroy_dialog (PPP_STATE_LISTENER (object));
}

static void
ppp_progress_state_listener_init(PppStateListenerIface *iface)
{
	iface->on_startup    = (PSLOnStartupFunc)   gtk_true;
	iface->on_tray_usage = (PSLOnTrayUsageFunc) ppp_progress_destroy_dialog;
	
	iface->on_idle       = ppp_progress_on_idle;
	iface->on_connected  = ppp_progress_on_connected;
	iface->on_connecting = ppp_progress_on_connecting;
}

static void ppp_progress_set_property(GObject * object, guint prop_id,
				  const GValue * value,
				  GParamSpec * pspec)
{
	if (PROP_MANAGER == prop_id)
		PPP_PROGRESS_GET_PRIVATE (object)->manager = g_value_get_object (value);
	else
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
}

/************ end of boiler plate code *******************/
static void
ppp_progress_destroy_dialog (PppStateListener * self)
{
	PppProgressPrivate *p = PPP_PROGRESS_GET_PRIVATE (self);
	/* check if we have the connecting dialog */
	if (p->dlg_connecting) {
		/* if we do we must destroy it and deref it */
		gtk_widget_destroy (p->dlg_connecting);
		p->dlg_connecting = NULL;
	}
}

static void
ppp_progress_on_connecting(PppStateListener * self,
				       GObject * origin)
{
	PppProgressPrivate *p = PPP_PROGRESS_GET_PRIVATE (self);
	gchar *msg;
	msg = g_strdup_printf ("<b><big>%s</big></b>\n\n%s", _("Connecting"), _("Establishing connection to server..."));
	p->dlg_connecting = gtk_message_dialog_new_with_markup (
		NULL,
		0,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CANCEL,
		msg
	);
	g_free (msg);
	g_signal_connect (p->dlg_connecting, "response", G_CALLBACK(ppp_progress_cancel), p);
	gtk_widget_show (p->dlg_connecting);
}

static void
ppp_progress_on_connected(PppStateListener * self,
				      GObject * origin,
				      gboolean new_connection)
{
	PppProgressPrivate *p = PPP_PROGRESS_GET_PRIVATE (self);
	gchar *msg;
	const gchar *title, *desc;
	GtkWidget *dialog;
	
	g_assert (p->manager);
	
	ppp_progress_destroy_dialog (self);
	title = _("Connected");
	if (new_connection)
		desc = _("Connection successfull.");
	else
		desc = _("Connection was already up.");
	
	if (ppp_manager_use_tray (p->manager)) {
		msg = g_strdup_printf (
			"<b><big>%s</big></b>\n\n%s %s",
			title,
			desc,
			_("Details are accessible through the notification icon.")
		);
	} else {
		msg = g_strdup_printf ("<b><big>%s</big></b>\n\n%s", title, desc);
	}
	
	dialog = gtk_message_dialog_new_with_markup (NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, msg);
	g_free (msg);
	
	g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
	gtk_widget_show (dialog);
}

static void
ppp_progress_cancel (GtkDialog *dialog, gint response, PppProgressPrivate *p)
{
	/* user pressed cancel */
	if (GTK_RESPONSE_CANCEL == response) {
		/* disconnect */
		ppp_manager_disconnect (p->manager, FALSE);
	}
}

static void
ppp_progress_on_idle (PppStateListener* self, GObject* origin)
{
	pppd_monitor *monitor;
	pppd_monitor_reason reason;
	PppManager *manager;
	
	ppp_progress_destroy_dialog (self);
	
	manager = PPP_PROGRESS_GET_PRIVATE (self)->manager;
	monitor =  ppp_manager_get_monitor (manager);
	reason = pppd_monitor_get_reason(monitor);
	
	if (PPPD_MONITOR_ERROR == reason) {
		const TGladeInfo nfo[] = {
			{"ppp_error_dialog", GTK_TYPE_DIALOG},
			{"log", GTK_TYPE_LABEL},
			{NULL}
		};
		GHashTable *widgets;
		GtkWidget *dialog;
		GtkLabel *label;
		GError *error = NULL;
		GString *buffer;
		GSList *iter;
		
		widgets = t_glade_parse (ppp_manager_get_glade_file (manager), NULL, nfo, NULL, &error);
		/* silently show a simple dialog, not showing the log */
		if (error) {
			T_ERR_WARN (error);
			t_error (NULL, _("Error trying to start connection."), "");
			return;
		}
		dialog = GTK_WIDGET (g_hash_table_lookup (widgets, "ppp_error_dialog"));
		label = GTK_LABEL (g_hash_table_lookup (widgets, "log"));
		buffer = g_string_new ("");

		for (iter = ppp_manager_get_log (manager); iter; iter = iter->next)
			g_string_append (buffer, iter->data);
		
		gtk_label_set_text (label, buffer->str);
		g_string_free (buffer, TRUE);
		
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}
	
}
