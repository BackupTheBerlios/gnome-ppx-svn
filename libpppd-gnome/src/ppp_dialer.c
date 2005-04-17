#include "ppp_dialer.h"

#include <glib.h>
#include <glib-object.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <glib/gi18n.h>
#include <string.h>
#include <tlib/tlib.h>
#include "ppp_marshallers.h"
#include "ppp_state_listener.h"
#include <config.h>
#include <pppd.h>
#include <prefix.h>
#include <unistd.h>
#include <sys/types.h>

#define PPP_GET_PASSWORD (BR_LIBEXECDIR("/ppp_get_password"))
#define PPP_DIALER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PPP_TYPE_DIALER, PppDialerPrivate))
typedef struct _PppDialerPrivate PppDialerPrivate;

static void ppp_dialer_finalize(GObject * object);
static void ppp_dialer_set_property(GObject * object, guint prop_id,
				      const GValue * value,
				      GParamSpec * pspec);
static void ppp_dialer_get_property(GObject * object, guint prop_id,
				      GValue * value, GParamSpec * pspec);
static GObject *
ppp_dialer_constructor (GType type,
                          guint n_props,
                          GObjectConstructParam *props);

static void ppp_dialer_state_listener_init(PppStateListenerIface *iface);
static gboolean ppp_dialer_username (GtkEntry *w, GdkEventFocus *event, PppDialerPrivate *p);
static gboolean ppp_dialer_password (GtkEntry *w, GdkEventFocus *event, PppDialerPrivate *p);
static void ppp_dialer_keep_password (GtkToggleButton *w, PppDialerPrivate *p);
static void ppp_dialer_connect (GtkWidget *w, PppDialerPrivate *p);
static gboolean ppp_dialer_quit (GtkWidget *w, PppDialerPrivate *p);
static void ppp_dialer_on_conf_update (GConfClient *c, guint conn_id, GConfEntry *e, PppDialerPrivate *p);
static void ppp_dialer_validate (GtkWidget *w, PppDialerPrivate *p);
static void ppp_dialer_to_gconf (PppDialerPrivate *p);
static gboolean ppp_dialer_delete (GtkWidget *w, GdkEvent *e, PppDialerPrivate *p);
static gboolean ppp_dialer_on_startup (PppStateListener* self, GObject* origin);
static void ppp_dialer_set_glade_file (PppDialer *self, const gchar *glade_file);
static void ppp_dialer_set_gconf_key (PppDialer *self, const gchar *gconf_key);
static void ppp_dialer_set_account (PppDialer *self, const gchar *account);
static gboolean ppp_dialer_check_deps (PppDialerPrivate *p);


enum {
	PROP_0,
	PROP_MANAGER,
	PROP_ACCOUNT,
};

struct _PppDialerPrivate {
	PppDialer *self;
	PppManager *manager;
	GConfClient *gconf;
	gchar *old_password;
	
	/* gconf keys */
	gchar *key_pppd_path;
	gchar *key_username;
	gchar *key_password;
	gchar *key_keep_password;
	
	/* widgets */
	GtkTable *table;
	GtkEntry *username;
	GtkEntry *password;
	GtkWidget *password_label;
	GtkToggleButton *keep_password;
	GtkWidget *connect;
	GtkLabel *title;
	GtkWidget *preferences;
	
	/* default pppd options */
	pppd_option *opt_account;
	pppd_option *opt_promptprog;
	
	/* helper memory allocators */
	TRefbag *strs_bag;
};

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (PppDialer, ppp_dialer, GTK_TYPE_WINDOW,
	G_IMPLEMENT_INTERFACE (
		PPP_TYPE_STATE_LISTENER,
		ppp_dialer_state_listener_init
	)
);

static void
ppp_dialer_class_init(PppDialerClass * klass)
{
	GObjectClass *gobject_class;

	klass->save_changes = NULL;
	klass->get_options = NULL;
	klass->connect_is_sensitive = NULL;
	klass->can_connect = NULL;
	
	gobject_class = G_OBJECT_CLASS(klass);
	
	g_type_class_add_private(gobject_class, sizeof(PppDialerPrivate));
	gobject_class->finalize = ppp_dialer_finalize;
	gobject_class->set_property = ppp_dialer_set_property;
	gobject_class->get_property = ppp_dialer_get_property;
	gobject_class->constructor = ppp_dialer_constructor;
	
	g_object_class_install_property (gobject_class, PROP_MANAGER,
					g_param_spec_object("manager",
							    "Dialer's manager",
							    "The dialer's manager",
							    PPP_TYPE_MANAGER,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
					gobject_class,
					PROP_ACCOUNT,
					g_param_spec_string (
								"account",
								"Account name",
								"pppd's account name",
								NULL,
								G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static GObject *
ppp_dialer_constructor (GType type,
                          guint n_props,
                          GObjectConstructParam *props)
{
	GObject *object;
	PppDialerClass *p_klass;
	
	object = G_OBJECT_CLASS (ppp_dialer_parent_class)->constructor (type, n_props, props);
	p_klass = PPP_DIALER_GET_CLASS (object);
	
	/* call the protected method for setting up the gui */
	if (p_klass->gui_init)
		p_klass->gui_init (PPP_DIALER (object));
	
	return object;
}

static void
ppp_dialer_set_property(GObject * object, guint prop_id,
				    const GValue * value,
				    GParamSpec * pspec)
{
	PppDialer *self;
	PppManager *manager;
	

	self = PPP_DIALER(object);
	switch (prop_id) {
		case PROP_MANAGER:
			manager = g_value_get_object (value);
			PPP_DIALER_GET_PRIVATE (self)->manager = manager;
			ppp_dialer_set_glade_file(self, ppp_manager_get_glade_file (manager));
			ppp_dialer_set_gconf_key (self, ppp_manager_get_gconf_dir (manager));
			break;
		case PROP_ACCOUNT:
			ppp_dialer_set_account (self, g_value_get_string (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
ppp_dialer_get_property(GObject * object, guint prop_id,
				    GValue * value, GParamSpec * pspec)
{
	PppDialer *self;

	self = PPP_DIALER(object);
	switch (prop_id) {
		case PROP_MANAGER:
			g_value_set_object(value, ppp_dialer_get_manager(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
ppp_dialer_init(PppDialer * object)
{
	PppDialerPrivate *p;
	p = PPP_DIALER_GET_PRIVATE(object);
	p->strs_bag = t_refbag_new_string();
	p->gconf = gconf_client_get_default();
	gtk_window_set_title (GTK_WINDOW (object), "GNOME PPX " VERSION);
	gtk_window_set_resizable (GTK_WINDOW (object), FALSE);

	/* init string references */
	t_refbag_add (p->strs_bag, &p->old_password);
	t_refbag_add (p->strs_bag, &p->key_pppd_path);
	t_refbag_add (p->strs_bag, &p->key_username);
	t_refbag_add (p->strs_bag, &p->key_password);
	t_refbag_add (p->strs_bag, &p->key_keep_password);
	
	p->self = object;
	p->opt_account = NULL;
	p->opt_promptprog = pppd_option_new ("promptprog");
	pppd_option_set_const_string (p->opt_promptprog, PPP_GET_PASSWORD);
	g_signal_connect (object, "delete-event", G_CALLBACK(ppp_dialer_delete), p);

}

static void
ppp_dialer_finalize(GObject * object)
{
	PppDialerPrivate *p = PPP_DIALER_GET_PRIVATE(object);
	/* free all private data */
	t_refbag_unref(p->strs_bag);
	if (p->opt_account)
		pppd_option_free (p->opt_account);
	pppd_option_free (p->opt_promptprog);
	/* chain up, since we are not deriving from a GObject */
	G_OBJECT_CLASS (ppp_dialer_parent_class)->finalize (object);
}

static void
ppp_dialer_state_listener_init(PppStateListenerIface *iface)
{
	iface->on_idle       = (PSLOnIdleFunc) gtk_widget_show;
	iface->on_connecting = (PSLOnConnectingFunc) gtk_widget_hide;
	iface->on_startup    = ppp_dialer_on_startup;
	iface->on_connected  = (PSLOnConnectedFunc) gtk_widget_hide;
	iface->on_tray_usage = (PSLOnTrayUsageFunc) gtk_false;
}

/****************** end of boiler plate code *******************/

static void
ppp_dialer_set_account (PppDialer *self, const gchar *account)
{
	PppDialerPrivate *p;
	if (!account)
		return;
	
	p = PPP_DIALER_GET_PRIVATE (self);
	p->opt_account = pppd_option_new ("call");
	pppd_option_set_const_string (p->opt_account, account);
}

static void
ppp_dialer_set_glade_file (PppDialer *self, const gchar *glade_file)
{
	PppDialerPrivate *p = PPP_DIALER_GET_PRIVATE(self);
	const TGladeInfo widgets[] = {
		{"ppp_dialer", GTK_TYPE_WIDGET},
		{"username", GTK_TYPE_ENTRY},
		{"password", GTK_TYPE_ENTRY},
		{"password_label", GTK_TYPE_LABEL},
		{"keep_password", GTK_TYPE_TOGGLE_BUTTON},
		{"connect", GTK_TYPE_BUTTON},
		{"table", GTK_TYPE_TABLE},
		{"title", GTK_TYPE_LABEL},
		{"preferences", GTK_TYPE_BUTTON},
		{NULL}
	};
	const TGladeSignal signals[] = {
		{"ppp_dialer_username", G_CALLBACK(ppp_dialer_username), (gpointer) p},
		{"ppp_dialer_password", G_CALLBACK(ppp_dialer_password), (gpointer) p},
		{"ppp_dialer_keep_password", G_CALLBACK(ppp_dialer_keep_password), (gpointer) p},
		{"ppp_dialer_connect", G_CALLBACK(ppp_dialer_connect), (gpointer) p},
		{"ppp_dialer_quit", G_CALLBACK(ppp_dialer_quit), (gpointer) p},
		{"ppp_dialer_validate", G_CALLBACK(ppp_dialer_validate), (gpointer) p},
		{NULL}
	};
	GHashTable *ht;
	GError *err = NULL;
	
	ht = t_glade_parse (glade_file, NULL, widgets, signals, &err);
	T_ERR_RETURN (err);

	/* bind widgets */
	gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (g_hash_table_lookup (ht, "ppp_dialer")));
	p->username       = GTK_ENTRY  (g_hash_table_lookup (ht, "username"));
	p->password       = GTK_ENTRY  (g_hash_table_lookup (ht, "password"));
	p->password_label = GTK_WIDGET (g_hash_table_lookup (ht, "password_label"));
	p->keep_password  = GTK_TOGGLE_BUTTON (g_hash_table_lookup (ht, "keep_password"));
	p->connect        = GTK_WIDGET (g_hash_table_lookup (ht, "connect"));
	p->table          = GTK_TABLE  (g_hash_table_lookup (ht, "table"));
	p->title          = GTK_LABEL  (g_hash_table_lookup (ht, "title"));
	p->preferences    = GTK_WIDGET (g_hash_table_lookup(ht, "preferences"));
	
	gtk_container_set_border_width (GTK_CONTAINER (self), 12);
	
	/* we don't need it any more */
	g_hash_table_destroy (ht);
}

static void
ppp_dialer_set_gconf_key (PppDialer *self, const gchar *gconf_key)
{
	PppDialerPrivate *p;
	GConfClientNotifyFunc f;
	gchar *str;
	gboolean b;
	GError *err = NULL;
	
	p = PPP_DIALER_GET_PRIVATE (self);
	/* setup gconf */
	p->key_username = g_strconcat(gconf_key, "/username", NULL);
	p->key_password = g_strconcat(gconf_key, "/password", NULL);
	p->key_keep_password = g_strconcat(gconf_key, "/keep_password", NULL);
	p->key_pppd_path = g_strconcat(gconf_key, "/pppd_path", NULL);
	/* now we set the password and keep_password in the env variables, so that
	 * ppp-get-password can know which are they.
	 */
	/* TODO: make a pretty window instead of bailing out? */
	g_return_if_fail(g_setenv("PPP_GCONF_PASSWORD", p->key_password, TRUE));
	g_return_if_fail(g_setenv("PPP_GCONF_KEEP_PASSWORD", p->key_keep_password, TRUE));
	
	gconf_client_add_dir (p->gconf, gconf_key, GCONF_CLIENT_PRELOAD_NONE, &err);
	T_ERR_RETURN (err);
	
	f = (GConfClientNotifyFunc) ppp_dialer_on_conf_update;
	gconf_client_notify_add (p->gconf, p->key_username, f, p, NULL, NULL);
	gconf_client_notify_add (p->gconf, p->key_password, f, p, NULL, NULL);
	gconf_client_notify_add (p->gconf, p->key_keep_password, f, p, NULL, NULL);
	/* get the default values */
	str = gconf_client_get_string(p->gconf, p->key_username, &err);
	T_ERR_WARN (err);
	if (str)
		gtk_entry_set_text(p->username, str);
	str = gconf_client_get_string(p->gconf, p->key_password, &err);
	T_ERR_WARN (err);
	if (str)
		gtk_entry_set_text(p->password, str);
	
	b = gconf_client_get_bool(p->gconf, p->key_keep_password, &err);
	T_ERR_WARN (err);
	
	/* generate the event */
	gtk_toggle_button_set_active(p->keep_password, b);
	ppp_dialer_keep_password(p->keep_password, p);
}

/****************** end of setters *****************************/
static gboolean
ppp_dialer_on_startup (PppStateListener* self, GObject* origin)
{
	PppDialerPrivate *p = PPP_DIALER_GET_PRIVATE (self);
	
	if (!ppp_dialer_check_deps (PPP_DIALER_GET_PRIVATE (self)))
		return FALSE;
	if (!ppp_dialer_check_app (p->key_pppd_path, "pppd"))
		return FALSE;
	ppp_dialer_update_connect_sensitivity (PPP_DIALER (self));
	return TRUE;
}

static gboolean
ppp_dialer_username (GtkEntry *username, GdkEventFocus *event, PppDialerPrivate *p)
{
	gconf_client_set_string (p->gconf, p->key_username, gtk_entry_get_text(username), NULL);
	return FALSE;
}

static gboolean
ppp_dialer_password (GtkEntry *password, GdkEventFocus *event, PppDialerPrivate *p)
{
	gconf_client_set_string (p->gconf, p->key_password, gtk_entry_get_text(password), NULL);
	return FALSE;
}

static void
ppp_dialer_keep_password (GtkToggleButton *keep_password, PppDialerPrivate *p)
{
	gboolean val = gtk_toggle_button_get_active(keep_password);
	gconf_client_set_bool(p->gconf, p->key_keep_password, val, NULL);
	/* let's save the old password for etiquete ;) */
	if (!val) {
		g_assert (!p->old_password);
		p->old_password = g_strdup(gtk_entry_get_text(p->password));
		gtk_entry_set_text(p->password, "");
		gconf_client_set_string(p->gconf, p->key_password, "", NULL);
	} else {
		if (p->old_password) {
			gtk_entry_set_text(p->password, p->old_password);
			gconf_client_set_string(p->gconf, p->key_password, p->old_password, NULL);
			g_free(p->old_password);
			p->old_password = NULL;
		}
	}
	gtk_widget_set_sensitive(GTK_WIDGET(p->password), val);
	gtk_widget_set_sensitive(p->password_label, val);
}

static void
ppp_dialer_on_conf_update (GConfClient *c, guint conn_id, GConfEntry *e, PppDialerPrivate *p)
{
	GtkEntry *entry = NULL;
	
	if (!strcmp(e->key, p->key_username)) {
		entry = p->username;
	} else if (!strcmp(e->key, p->key_password)) {
		entry = p->password;
	} else if (!strcmp(e->key, p->key_keep_password)) {
		gtk_toggle_button_set_active(p->keep_password, gconf_value_get_bool(e->value));
	}
	
	if (entry) {
		gtk_entry_set_text(entry, gconf_value_get_string(e->value));
	}
}

/* checks some applications which gnome-ppx depends */
static gboolean
ppp_dialer_check_deps (PppDialerPrivate *p)
{
	GError *error = NULL;
	
	if (!t_update_program_in_gconf ("pppd", p->key_pppd_path, &error)) {
		t_error (
			GTK_WINDOW (p->self),
			_("Could not find <i>pppd</i> program"),
			error->message
		);
		g_error_free (error);
		return FALSE;
	}
	
	if (!g_file_test(PPP_GET_PASSWORD, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_EXECUTABLE)) {
		t_error (
			GTK_WINDOW (p->self),
			_("Installation corrupted"),
			_("Please reinstall GNOME PPX, there are some files missing.")
		);
		return FALSE;
	}
	
	return TRUE;
}

static void
ppp_dialer_connect (GtkWidget *w, PppDialerPrivate *p)
{
	gchar *pppd, *username;
	PppDialerClass *klass;
	GError *err = NULL;
	GSList *options;
	
	/* check preconditions */
	klass = PPP_DIALER_GET_CLASS(p->self);
	ppp_dialer_to_gconf(p);
	
	if (!ppp_dialer_check_deps (p))
		return;
	
	username = gconf_client_get_string(p->gconf, p->key_username, &err);
	T_ERR_WARN (err);
	if (!username) {
		t_error (
			GTK_WINDOW (p->self),
			_("Could not get username"),
			_("There was an error getting the entry from configuration storage.")
		);
		return;
	}
	
	pppd = gconf_client_get_string(p->gconf, p->key_pppd_path, &err);
	T_ERR_WARN (err);
	if (!pppd) {
		t_error (
			GTK_WINDOW (p->self),
			_("Could not find <i>pppd</i> program"),
			_("There was an error getting the entry from configuration storage.")
		);
		return;
	}
	
	if (klass->can_connect && !klass->can_connect (p->self))
		return;
	/* hide window and start status */
	g_return_if_fail (klass->get_options);
	options = klass->get_options(p->self);
	
	options = g_slist_prepend (options, p->opt_promptprog);
	if (p->opt_account)
		options = g_slist_prepend (options, p->opt_account);
	
	ppp_manager_call (p->manager, pppd, username, options);
	
	options = g_slist_remove (options, p->opt_promptprog);
	if (p->opt_account)
		g_slist_remove (options, p->opt_account);
}

static gboolean
ppp_dialer_quit (GtkWidget *w, PppDialerPrivate *p)
{
	/*
	 * If you have an entry selected and click on a button
	 * really fast it wont accept the focus out event.
	 * Therefore we have to make sure all changes are saved.
	 */
	ppp_dialer_to_gconf(p);
	/* because all changes are saved instantly there's no need to confirm */
	return ppp_manager_quit (p->manager, FALSE);
}

/*
 * Saves current entries text of entries to gconf backend.
 */
static void
ppp_dialer_to_gconf (PppDialerPrivate *p)
{
	const gchar *text;
	PppDialerClass *klass;
	klass = PPP_DIALER_GET_CLASS (p->self);
	
	text = gtk_entry_get_text(p->username);
	gconf_client_set_string(p->gconf, p->key_username, text, NULL);
	text = gtk_entry_get_text(p->password);
	gconf_client_set_string(p->gconf, p->key_password, text, NULL);
	
	if (klass->save_changes)
		klass->save_changes (p->self);
}

/*
 * Makes the connect button unsensitive when all the fields are not met.
 * Should we add a status bar explaining the reason?
 */
static void
ppp_dialer_validate (GtkWidget *w, PppDialerPrivate *p)
{
	ppp_dialer_update_connect_sensitivity(p->self);
}

GtkTable *
ppp_dialer_get_table(PppDialer *self)
{
	return PPP_DIALER_GET_PRIVATE(self)->table;
}

void
ppp_dialer_update_connect_sensitivity (PppDialer *self)
{
	int ret;
	PppDialerPrivate *p = PPP_DIALER_GET_PRIVATE(self);
	PppDialerClass *klass = PPP_DIALER_GET_CLASS (self);
	ret = strlen(gtk_entry_get_text(p->username));
	ret = ret && (strlen(gtk_entry_get_text(p->password)) || !gtk_toggle_button_get_active(p->keep_password));
	/* don't force user to implement this */
	ret = ret && (!klass->connect_is_sensitive || klass->connect_is_sensitive (self));
	gtk_widget_set_sensitive(p->connect, ret);
}

void
ppp_dialer_set_title (PppDialer *self, const gchar *title)
{
	PppDialerPrivate *p = PPP_DIALER_GET_PRIVATE(self);
	gchar *tmp;
	
	tmp = g_strconcat("<b><big>", title, "</big></b>", NULL);
	gtk_label_set_markup(p->title, tmp);
	g_free(tmp);
}

/* called when user tries to close window */
static gboolean
ppp_dialer_delete (GtkWidget *w, GdkEvent *e, PppDialerPrivate *p)
{
	/* since all changes are reflected instantly there's no problem not asking
	 * if the user wants to quit.
	 *
	 * returning FALSE lets the widget destroy it self,
	 * this is why we negate the condition.
	 */
	return !ppp_dialer_quit (w, p);
}

GtkWidget *
ppp_dialer_get_preferences_button (PppDialer *self)
{
	g_assert(PPP_IS_DIALER(self));
	g_assert(PPP_DIALER_GET_PRIVATE(self)->preferences);
	return PPP_DIALER_GET_PRIVATE(self)->preferences;
}

PppManager *
ppp_dialer_get_manager (PppDialer *self)
{
	return PPP_DIALER_GET_PRIVATE (self)->manager;
}

gboolean
ppp_dialer_check_app (const gchar *gconf_key, const gchar *pretty_name)
{
	gchar *app;
	GConfClient *gconf;
	GError *err = NULL;
	const gchar *app_name = pretty_name;
	gboolean has_perms;
	
	gconf = gconf_client_get_default ();
	
	app = gconf_client_get_string(gconf, gconf_key, &err);
	if (!app_name)
		app_name = g_basename (app);
	has_perms = getuid() == 0 || (getuid() != 0 && t_has_suid (app));
	
	T_ERR_WARN (err);
	
	if (!has_perms) {
		gchar *argv_0;
		gchar *desc = g_strdup_printf (_("You are not root and <i>%s</i> does not have suid bit set."), app_name);
		t_warn (NULL, _("Insuficient privileges"), desc);
		g_free (desc);
		argv_0 = g_get_prgname ();
		t_su_exec (argv_0, NULL);
		g_free (argv_0);
	}
	return has_perms;
}
