#include "tsu-gksu.h"
#include <gtk/gtk.h>
#include <glib/gi18n.h>

/* gksu iface */
static gpointer (*gksu_new)          (void);
static void     (*gksu_set_password) (gpointer gksu, const gchar *password);
static void     (*gksu_set_command)  (gpointer gksu, const gchar *command);
static gboolean (*gksu_run)          (gpointer gksu, GError **error);
static void     (*gksu_free)         (gpointer gksu);
static void     (*gksu_set_user)     (gpointer gksu, const gchar *user);

/* gksuui iface */
static GtkWidget* (*gksuui_new) (void);
static gchar*     (*gksuui_get_password) (GtkWidget *w);
static void       (*gksuui_set_message)  (GtkWidget *w, const gchar *message);

static TSymbol gksu_iface[] = {
	{"gksu_context_new", (gpointer *) &gksu_new},
	{"gksu_context_set_user", (gpointer *) &gksu_set_user},
	{"gksu_context_set_password", (gpointer *) &gksu_set_password},
	{"gksu_context_set_command", (gpointer *) &gksu_set_command},
	{"gksu_context_run", (gpointer *) &gksu_run},
	{"gksu_context_free", (gpointer *) &gksu_free},
	{NULL}
};

static TSymbol gksuui_iface[] = {
	{"gksuui_dialog_new", (gpointer *) &gksuui_new},
	{"gksuui_dialog_get_password", (gpointer *) &gksuui_get_password},
	{"gksuui_dialog_set_message", (gpointer *) &gksuui_set_message},
	{NULL}
};

static TModule mods[] = {
	{"gksu1.2", gksu_iface, NULL},
	{"gksuui1.0", gksuui_iface, NULL},
	{NULL}
};

static gboolean t_gksu_exec (const gchar *command, GError **error)
{
	gpointer gksu;
	GtkWidget *dlg;
	gboolean ret;
	gchar *pwd;
	
	dlg = gksuui_new ();
	
	if (gtk_dialog_run (GTK_DIALOG (dlg)) != GTK_RESPONSE_OK) {
		if (error) {
			*error = g_error_new (
				T_SU_ERROR,
				T_SU_ERROR_UNKNOWN,
				_("Could not run command '%s' as a different user"),
				command
			);
		}
		return FALSE;
	}
	gtk_widget_hide (dlg);
	pwd = gksuui_get_password (dlg);
	
	gtk_widget_destroy (dlg);
	
	
	gksu = gksu_new ();
	gksu_set_password (gksu, pwd);
	g_free (pwd);
	gksu_set_command (gksu, command);
	gksu_set_user (gksu, "root");
	
	ret = gksu_run (gksu, NULL);
	
	if (!ret && error) {
		*error = g_error_new (
			T_SU_ERROR,
			T_SU_ERROR_UNKNOWN,
			_("Could not run command '%s' as a different user"),
			command
		);
	}
	gksu_free (gksu);
	return ret;
}

const TSuModule *
t_gksu_module (void)
{
	static TSuModule su_mod = {mods, t_gksu_exec};
	return &su_mod;
}
