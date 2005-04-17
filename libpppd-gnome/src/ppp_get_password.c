
#include <glib.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gnome.h>
#include <gconf/gconf-client.h>
#include <libgnome/gnome-i18n.h>
#include <prefix.h>

GtkEntry *password_entry;

const gchar *username;
const gchar *password;
const gchar *server;

int ret;
int output_file_descriptor;

void change_gtk_label(GtkLabel * gtk_label)
{
	gchar *message;
	const gchar *format;

	format =
	    L_("Enter the password for user <b>%s</b> on server <b>%s</b>");

	if (!strlen(username) && !strlen(server))
		format = L_("Enter the account password%s%s");
	else if (!strlen(username))
		format = L_("Enter the password for server <b>%s%s</b>");
	else if (!strlen(server))
		format = L_("Enter the password for user <b>%s%s</b>");

	message = g_strdup_printf(format, username, server);

	gtk_label_set_markup(GTK_LABEL(gtk_label), message);

	g_free(message);
}

void on_btn_ok_clicked(GtkWidget * button, gpointer user_data)
{
	GtkWidget *gtk_entry;

	if (gtk_entry) {
		int len = 0;

		password = gtk_entry_get_text(password_entry);
		len = strlen(password);

		if (write(output_file_descriptor, password, len) != len) {
			g_warning(L_("could not write to file descriptor\n"));
		}
	}

	ret = 0;
	gtk_main_quit();
}

void on_btn_cancel_clicked(GtkWidget * button, gpointer user_data)
{
	ret = 1;
	gtk_main_quit();
}

gboolean
on_gtk_entry_password_key_press_event(GtkWidget * entry,
				      GdkEventKey * event,
				      gpointer user_data)
{
	switch (event->keyval) {
	case GDK_Return:
		on_btn_ok_clicked(NULL, NULL);
		break;

	case GDK_Escape:
		on_btn_cancel_clicked(NULL, NULL);
		break;

	default:
		break;
	}

	return FALSE;
}

int main(int argc, char *argv[])
{
	if (argc < 4) {
		g_critical(L_("Expecting 3 arguments."));
		return 1;
	} else {
		GtkLabel *lbl;
		GladeXML *glade_xml;
		GError *err = NULL;
		GConfClient *gconf;
		gboolean keep_password;
		gchar *password;
		const gchar *gconf_keep_password;
		const gchar *gconf_password;
		gtk_init(&argc, &argv);
		gconf_password = g_getenv("PPP_GCONF_PASSWORD");
		gconf_keep_password = g_getenv("PPP_GCONF_KEEP_PASSWORD");
		if (!gconf_password || !gconf_keep_password) {
			g_critical(L_("Environment variables not set."));
			exit(1);
		}
		if (argv[3] != NULL) {
			output_file_descriptor = atoi(argv[3]);
		}
		gconf = gconf_client_get_default();
		keep_password = gconf_client_get_bool(gconf, gconf_keep_password, &err);
		
		/* user has password in database, spit it out */
		if (!err && keep_password) {
			password = gconf_client_get_string(gconf, gconf_password, &err);
			if (!err && password) {
				int len = strlen(password);
			
				if (write(output_file_descriptor, password, len) != len) {
					g_warning(L_("could not write to file descriptor\n"));
				}
				/* all went fine, say bye */
				return 0;
			}
		}
		
		glade_init();
		glade_xml =
		    glade_xml_new(BR_DATADIR("/gnome-ppx/ppp_get_password.glade"), NULL,
				  NULL);
		glade_xml_signal_autoconnect(glade_xml);

		password_entry =
		    GTK_ENTRY(glade_xml_get_widget
			      (glade_xml, "gtk_entry_password"));
		lbl =
		    GTK_LABEL(glade_xml_get_widget
			      (glade_xml, "msg_label"));
		g_object_unref(glade_xml);

		username = argv[1];
		server = argv[2];

		change_gtk_label(lbl);
		gtk_main();
	}

	return ret;
}
