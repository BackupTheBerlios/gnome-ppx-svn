/*
 * We construct the manager like: m = new Manager();
 * then we must set a new dialer: m.dialer = new PppoeDialer();
 * after this we run the dialer: m.run();
 * During the program, if we are not connected we can issue a: m.call (some args);
 */
#include "ppp_manager.h"
#include "ppp_state_listener.h"
#include "ppp_tray.h"
#include "ppp_progress.h"
#include <tlib/tlib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib-object.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _PppManagerPrivate PppManagerPrivate;
#define PPP_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PPP_TYPE_MANAGER, PppManagerPrivate))

static void ppp_manager_finalize(GObject * object);
static void ppp_manager_set_property(GObject * object, guint prop_id,
				     const GValue * value,
				     GParamSpec * pspec);
static void ppp_manager_get_property(GObject * object, guint prop_id,
				     GValue * value, GParamSpec * pspec);
static void ppp_manager_change_state (PppManagerPrivate *p, gint state);
static void ppp_manager_about (PppTray *tray, GtkWidget *origin, PppManagerPrivate *p);
static void ppp_manager_monitor (pppd_monitor *mon, void *data);
static int ppp_manager_exec (char * const args[], void * user_data);
static int ppp_manager_kill (int pid, void *user_data);
static void ppp_manager_on_conf_update (GConfClient *c, guint conn_id, GConfEntry *e, PppManagerPrivate *p);
static gboolean ppp_manager_on_state (gpointer data);
static void ppp_manager_set_link_name (PppManager * self, const gchar *link_name);
static void ppp_manager_set_glade_file(PppManager * self, const gchar * glade_file);
static void ppp_manager_set_gconf_dir (PppManager * self, const gchar *gconf_dir);
static void ppp_manager_clear_log (PppManagerPrivate *p);

struct _PppManagerPrivate {
	PppManager *self;
	
	GList *listeners;
	PppDialer *dialer;

	pppd_monitor *monitor;
	GAsyncQueue *states;
	gboolean new_connection;
	GConfClient *gconf;
	
	/* tray window */
	gchar *gconf_dir;
	gchar *glade_file;
	gchar *key_use_tray;
	gboolean use_tray;
	
	GSList *log;
};

enum {
	PROP_0,
	PROP_GLADE_FILE,
	PROP_LINK_NAME,
	PROP_DIALER,
	PROP_GCONF_DIR,
	PROP_1
};

enum {
	ON_IDLE,
	ON_CONNECTING,
	ON_CONNECTED,
	ON_INIT,
	ON_TRAY_USAGE
};

enum {
	ABOUT,
	LAST_SIGNAL
};

static pppd_monitor_callbacks cb = {
	ppp_manager_monitor,
	ppp_manager_exec,
	ppp_manager_kill
};

static guint signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE (PppManager, ppp_manager, G_TYPE_OBJECT);

static void ppp_manager_class_init(PppManagerClass * klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(gobject_class, sizeof(PppManagerPrivate));
	gobject_class->finalize = ppp_manager_finalize;
	gobject_class->set_property = ppp_manager_set_property;
	gobject_class->get_property = ppp_manager_get_property;
	
	g_object_class_install_property (gobject_class, PROP_GLADE_FILE,
					g_param_spec_string("glade-file",
							    "glade file",
							    "glade file",
							    NULL,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (gobject_class, PROP_LINK_NAME,
					g_param_spec_string("link-name",
							    "link_name",
							    "link_name",
							    NULL,
							    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (gobject_class, PROP_GCONF_DIR,
					g_param_spec_string("gconf-dir",
							    "GConf directory",
							    "GConf directory",
							    NULL,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (gobject_class, PROP_DIALER,
					g_param_spec_object("dialer",
								"Dialer widget",
								"Dialer widget",
								PPP_TYPE_DIALER,
								G_PARAM_READWRITE));
	signals[ABOUT] = g_signal_new(
		"about",
		G_OBJECT_CLASS_TYPE(gobject_class),
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET (PppManagerClass, about),
		NULL, NULL,
		g_cclosure_marshal_VOID__OBJECT,
		G_TYPE_NONE,    /* returns void */
		1,              /* has one argument */
		GTK_TYPE_WIDGET /* which is a widget */
	);
}

PppManager *ppp_manager_new(const gchar *glade_file, const gchar *link_name, const gchar *gconf_dir)
{
	PppManager *self;
	PppManagerPrivate *p;
	
	self = PPP_MANAGER(g_object_new(
		PPP_TYPE_MANAGER,
		"link-name", link_name,
		"glade-file", glade_file,
		"gconf-dir", gconf_dir,
		NULL
	));
	
	p = PPP_MANAGER_GET_PRIVATE (self);
	
	return self;
}

static void ppp_manager_init(PppManager * object)
{
	PppManagerPrivate *p;

	p = PPP_MANAGER_GET_PRIVATE(object);
	p->dialer = NULL;
	p->listeners = NULL;
	p->self = object;
	p->gconf = gconf_client_get_default();
	p->states = g_async_queue_new ();
	p->log = NULL;
}

static void ppp_manager_finalize(GObject * object)
{
	PppManagerPrivate *p = PPP_MANAGER_GET_PRIVATE (object);
	g_free (p->glade_file);
	g_free (p->gconf_dir);
	g_free (p->key_use_tray);
	pppd_monitor_free (p->monitor);
	g_async_queue_unref (p->states);
	
	/* make sure dialer is destroyed on exit */
	if (p->dialer)
		gtk_widget_destroy (GTK_WIDGET (p->dialer));
	
	/* unref listeners */
	g_list_foreach (p->listeners, (GFunc) g_object_unref, NULL);
	g_list_free (p->listeners);
	
	ppp_manager_clear_log (p);
}

static void ppp_manager_set_property(GObject * object, guint prop_id,
				     const GValue * value,
				     GParamSpec * pspec)
{
	PppManager *self;

	self = PPP_MANAGER(object);
	switch (prop_id) {
		case PROP_GLADE_FILE:
			ppp_manager_set_glade_file(self,
						   g_value_get_string(value));
			break;
		
		case PROP_LINK_NAME:
			ppp_manager_set_link_name (self, g_value_get_string (value));
			break;
		
		case PROP_DIALER:
			ppp_manager_set_dialer (self, g_value_get_object (value));
			break;
		
		case PROP_GCONF_DIR:
			ppp_manager_set_gconf_dir (self, g_value_get_string (value));
			break;
		
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void ppp_manager_get_property(GObject * object, guint prop_id,
				     GValue * value, GParamSpec * pspec)
{
	PppManager *self;

	self = PPP_MANAGER(object);
	switch (prop_id) {
		case PROP_DIALER:
			g_value_set_object (value, ppp_manager_get_dialer (self));
			break;
		
		case PROP_GLADE_FILE:
			g_value_set_string (value, ppp_manager_get_glade_file (self));
			break;
		
		case PROP_GCONF_DIR:
			g_value_set_string (value, ppp_manager_get_gconf_dir (self));
			break;
		
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

/**************** end of boiler plate code *************************/

static void ppp_manager_set_glade_file(PppManager * self, const gchar * glade_file)
{
	PppManagerPrivate *p = PPP_MANAGER_GET_PRIVATE(self);
	PppStatus *status;
	gpointer tray;
	
	p->glade_file = g_strdup (glade_file);
	
	status = PPP_STATUS (ppp_status_new (self));
	p->listeners = g_list_append (p->listeners, status);
	tray = ppp_tray_new (self, status);
	p->listeners = g_list_append (p->listeners, tray);
	g_signal_connect (tray, "about", G_CALLBACK (ppp_manager_about), p);
	p->listeners = g_list_append (p->listeners, ppp_progress_new (self));
}

static void ppp_manager_set_link_name (PppManager * self, const gchar *link_name)
{
	PppManagerPrivate *p = PPP_MANAGER_GET_PRIVATE(self);
	p->monitor = pppd_monitor_new (link_name, p, &cb);
	g_assert (p->monitor);
}

static void ppp_manager_set_gconf_dir (PppManager * self, const gchar *gconf_dir)
{
	PppManagerPrivate *p = PPP_MANAGER_GET_PRIVATE(self);
	GError *err = NULL;
	
	p->key_use_tray = g_strconcat (gconf_dir, "/use_tray", NULL);
	p->gconf_dir = g_strdup (gconf_dir);
	
	p->use_tray = gconf_client_get_bool (p->gconf, p->key_use_tray, &err);
	T_ERR_WARN (err);
	gconf_client_notify_add (
		p->gconf,
		p->key_use_tray,
		(GConfClientNotifyFunc)ppp_manager_on_conf_update,
		p,
		NULL,
		NULL
	);
}

static void
ppp_manager_on_dialer_finalize (PppManager *self, PppDialer *dialer)
{
	PppManagerPrivate *p = PPP_MANAGER_GET_PRIVATE (self);
	g_assert (p->dialer == dialer);
	g_list_remove (p->listeners, dialer);
	p->dialer = NULL;
}

void ppp_manager_set_dialer (PppManager *self, PppDialer *dialer)
{
	PppManagerPrivate *p = PPP_MANAGER_GET_PRIVATE(self);
	
	if (p->dialer) {
		g_object_unref (p->dialer);
		/* remove the monitor */
		g_object_weak_unref (G_OBJECT (p->dialer), (GWeakNotify) ppp_manager_on_dialer_finalize, self);
		/* remove listener from list */
		p->listeners = g_list_remove (p->listeners, p->dialer);
		
	}
	if (dialer) {
		g_object_ref (dialer);
		/* monitor dialer's life cycle */
		g_object_weak_ref (G_OBJECT (dialer), (GWeakNotify) ppp_manager_on_dialer_finalize, self);
		p->listeners = g_list_append (p->listeners, dialer);
	}
	p->dialer = dialer;
}

PppDialer* ppp_manager_get_dialer (PppManager *self)
{
	return PPP_DIALER (PPP_MANAGER_GET_PRIVATE (self)->dialer);
}

pppd_monitor* ppp_manager_get_monitor (PppManager * self)
{
	return PPP_MANAGER_GET_PRIVATE (self)->monitor;
}
/************* end of getter/setters ****************/

static gboolean ppp_manager_on_state (gpointer data)
{
	PppManagerPrivate *p = (PppManagerPrivate *) data;
	pppd_monitor_state s;
	gint state;
	/* grab current state */
	g_return_val_if_fail(g_async_queue_length (p->states), FALSE);
	s = (pppd_monitor_state) GPOINTER_TO_INT(g_async_queue_pop(p->states));
	if (PPPD_MONITOR_IDLE == s) {
		state = ON_IDLE;
	} else if (PPPD_MONITOR_CONNECTING == s) {
		state =  ON_CONNECTING;
	} else if (PPPD_MONITOR_CONNECTED == s) {
		state = ON_CONNECTED;
	}
	/* route state to listeners */
	ppp_manager_change_state (p, state);
	return FALSE;
}

static void ppp_manager_monitor (pppd_monitor *mon, void *data)
{
	PppManagerPrivate *p = (PppManagerPrivate *) data;
	g_return_if_fail (p);
	g_return_if_fail (p->monitor);
	/* push current state to queue */
	g_async_queue_push(p->states, GINT_TO_POINTER(pppd_monitor_get_state(p->monitor)));
	/* now start a g_idle call */
	g_idle_add (ppp_manager_on_state, data);
}

static void ppp_manager_on_conf_update (GConfClient *c, guint conn_id, GConfEntry *e, PppManagerPrivate *p)
{
	if (!strcmp(e->key, p->key_use_tray)) {
		p->use_tray = gconf_value_get_bool(e->value);
		ppp_manager_change_state (p, ON_TRAY_USAGE);
	}
}

static void ppp_manager_about (PppTray *tray, GtkWidget *origin, PppManagerPrivate *p)
{
	g_signal_emit (p->self, signals[ABOUT], 0, origin);
}

static int ppp_manager_kill (int pid, void *user_data)
{
	char buffer[1024];
	snprintf (buffer, sizeof(buffer), "kill -9 %d", pid);
	system (buffer);
	return 1;
}

static int ppp_manager_exec (char * const args[], void * user_data)
{
	GPid pid;
	GError *error = NULL;
	int stdout, stderr;
	PppManagerPrivate *p = (PppManagerPrivate *) user_data;
	g_assert (p);
	g_assert (!p->log);
	
	/* because argv[0] is pppd we don't need the file entry */
	g_spawn_async_with_pipes (
		NULL,                      /* working dir */
		(gchar **) args,           /* argv        */
		NULL,                      /* envp        */
		G_SPAWN_DO_NOT_REAP_CHILD, /* flags       */
		NULL,                      /* child setup */
		NULL,                      /* user data   */
		&pid,                      /* pid         */
		NULL,                      /* stdin       */
		&stdout,                   /* stdout      */
		&stderr,                   /* stderr      */
	    &error
	);
	T_ERR_RETURN_VAL (error, 0);
	t_fd_to_string (&p->log, stdout);
	t_fd_to_string (&p->log, stderr);
	return pid;
}

static void ppp_manager_change_state (PppManagerPrivate *p, gint state)
{
	GList *iter;
	
	switch (state) {
		case ON_IDLE:
			g_list_foreach (p->listeners, (GFunc) ppp_state_listener_on_idle, p->self);
			/* clean up log after we start call listeners */
			ppp_manager_clear_log (p);
			break;
		case ON_CONNECTING:
			g_list_foreach (p->listeners, (GFunc) ppp_state_listener_on_connecting, p->self);
			break;
		case ON_CONNECTED:
			for (iter = g_list_first (p->listeners); iter; iter = iter->next)
				ppp_state_listener_on_connected (PPP_STATE_LISTENER (iter->data), G_OBJECT(p->self), p->new_connection);
			/* we are connected, log is no longer needed */
			ppp_manager_clear_log (p);
			break;
		case ON_TRAY_USAGE:
			for (iter = g_list_first (p->listeners); iter; iter = iter->next)
				ppp_state_listener_on_tray_usage (PPP_STATE_LISTENER (iter->data), G_OBJECT(p->self), p->use_tray);
			break;
		default:
			g_return_if_reached ();
	}
}

/************ end of private methods **************/

gboolean
ppp_manager_quit (PppManager *self, gboolean confirm)
{
	PppManagerPrivate *p = PPP_MANAGER_GET_PRIVATE (self);
	GtkWidget *w;
	gboolean will_quit = TRUE;
	
	if (confirm) {
		w = gtk_message_dialog_new_with_markup(
			GTK_WINDOW(p->dialer),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING,
			GTK_BUTTONS_OK_CANCEL,
			"<b><big>%s</big></b>\n\n%s",
			_("Connection will not be terminated."),
			_("Closing the application will only stop monitoring the connection, not terminate it. "
			  "You can resume the connection monitoring by restarting this dialer.")
		);
		gtk_window_set_modal (GTK_WINDOW (w), TRUE);
		will_quit = gtk_dialog_run(GTK_DIALOG(w)) == GTK_RESPONSE_OK;
		gtk_widget_destroy(w);
	}
	
	if (will_quit) {
		pppd_monitor_detach (p->monitor);
		gtk_main_quit ();
	}
	
	return will_quit;
}

gboolean ppp_manager_disconnect (PppManager *self, gboolean confirm)
{
	PppManagerPrivate *p;
	GtkWidget *dlg;
	gboolean will_quit = TRUE;
	
	g_return_val_if_fail (PPP_IS_MANAGER (self), FALSE);
	
	p = PPP_MANAGER_GET_PRIVATE (self);
	if (confirm) {
		gchar *msg;
		msg = g_strdup_printf ("<b><big>%s</big></b>", _("Are you sure you want to terminate the connection?"));
		dlg = gtk_message_dialog_new_with_markup (
			NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
			GTK_MESSAGE_WARNING,
			GTK_BUTTONS_OK_CANCEL,
			msg
		);
		g_free (msg);
		gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
		will_quit = gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK;
		gtk_widget_destroy(dlg);
	}
	
	if (will_quit) {
		pppd_monitor_abort(p->monitor, NULL);
	}
	
	return will_quit;
}

gboolean ppp_manager_call (PppManager * self, const gchar *pppd, const gchar *username, GSList *opts)
{
	GPtrArray *argv;
	PppManagerPrivate *p;
	char *buffer[2];
	gboolean ret;
	int i, len;
	
	p = PPP_MANAGER_GET_PRIVATE(self);
	p->new_connection = TRUE;
	g_assert (username);
	/* pppd, 'user', the username, the opts and finally NULL value. */
	argv = g_ptr_array_sized_new (g_slist_length (opts) + 2 + 1 + 1);
	
	/* argv[0] must be pppd */
	g_ptr_array_add (argv, strdup (pppd));
	
	/* append username directive */
	g_ptr_array_add (argv, strdup ("user"));
	g_ptr_array_add (argv, strdup (username));
	
	/* append user options */
	for (; opts; opts = opts->next) {
		/* every directive MUST be correct */
		g_assert(pppd_option_to_str ((pppd_option *) opts->data, buffer));
		if (buffer[0]) {
			g_ptr_array_add (argv, buffer[0]);
			if (buffer[1])
				g_ptr_array_add (argv, buffer[1]);
		}
	}
	
	/* append null to terminate the array */
	g_ptr_array_add (argv, NULL);
	
	/* call pppd */
	g_print ("Running pppd: ");
	for (i = 0; argv->pdata[i]; i++)
		g_print ("\"%s\" ", (const gchar *) argv->pdata[i]);
	g_print ("\n");
	
	ret = pppd_monitor_call(p->monitor, pppd, (char **) argv->pdata, p);
	
	/* free each element */
	len = argv->len - 1;
	for (i = 0; i < len; i++)
		free (argv->pdata[i]);
	/* free argv */
	g_ptr_array_free (argv, TRUE);
	
	return ret;
}

gboolean ppp_manager_run(PppManager* self)
{
	PppManagerPrivate *p;
	p = PPP_MANAGER_GET_PRIVATE (self);
	GList *iter;
	gboolean can_start = TRUE;
	
	/* make sure we have a dialer */
	g_return_val_if_fail (ppp_manager_get_dialer (self), FALSE);
	
	p->new_connection = FALSE;
	
	/* check if we can go on */
	for (iter = g_list_first (p->listeners); iter && can_start; iter = iter->next) {
		PPP_STATE_LISTENER (iter->data);
		can_start = can_start && ppp_state_listener_on_startup (PPP_STATE_LISTENER (iter->data), G_OBJECT(p->self));
	}
	
	if (!can_start)
		return FALSE;
	g_assert (p->monitor);
	if (!pppd_monitor_attach (p->monitor))
		ppp_manager_change_state (p, ON_IDLE);
	
	return TRUE;
}

gboolean ppp_manager_use_tray (PppManager *self)
{
	g_return_val_if_fail (PPP_IS_MANAGER (self), FALSE);
	return PPP_MANAGER_GET_PRIVATE (self)->use_tray;
}

const gchar *ppp_manager_get_gconf_dir (PppManager *self)
{
	return PPP_MANAGER_GET_PRIVATE (self)->gconf_dir;
}

const gchar *ppp_manager_get_glade_file (PppManager *self)
{
	return PPP_MANAGER_GET_PRIVATE (self)->glade_file;
}

static void
ppp_manager_clear_log (PppManagerPrivate *p)
{
	/* free log data */
	if (p->log) {
		g_slist_foreach (p->log, (GFunc) g_free, NULL);
		g_slist_free (p->log);
		p->log = NULL;
	}
}

GSList *
ppp_manager_get_log (PppManager *self)
{
	return PPP_MANAGER_GET_PRIVATE (self)->log;
}
