#include "pptp_dialer.h"
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/gnome-i18n.h>
#include <prefix.h>
#include <config.h>

int main(int argc, char **argv)
{
	PppDialer *d;
	PppManager *m;
	GnomeProgram *app;
	gchar *glade_file;
	gchar *tmp_path, *path;

	bindtextdomain (PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain (PACKAGE);

	app = gnome_program_init("PPTP", VERSION, LIBGNOMEUI_MODULE, argc, argv, 
                   NULL, NULL);
	
	gtk_init(NULL, NULL);
	glade_init();
	
	/* make sure we have sbin in the path */
	path = g_strdup (g_getenv ("PATH"));
	if (!path)
		path = g_strdup ("/usr/bin:/usr/local/bin");
	tmp_path = g_strconcat (path, ":/usr/sbin:/usr/local/sbin", NULL);
	g_free (path);
	g_setenv ("PATH", tmp_path, TRUE);
	g_free (tmp_path);
	
	glade_file = g_strdup(BR_DATADIR("/gnome-ppx/libpppd-gnome.glade"));
	m = ppp_manager_new (glade_file, "pptp", "/apps/gnome-pptp");
	d = PPP_DIALER (pptp_dialer_new (m));
	g_free (glade_file);
	
	ppp_manager_set_dialer (m, d);
	g_object_unref (d);
	
	if (ppp_manager_run (m))
		gtk_main();
	
	g_object_unref (m);
	
	return 0;
}
