#include "pppoa_dialer.h"
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/gnome-i18n.h>
#include <prefix.h>
#include <config.h>
#include <tlib/tlib.h>
#include <unistd.h>
#include <sys/types.h>

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
		
	app = gnome_program_init("PPPoA", VERSION, LIBGNOMEUI_MODULE, argc, argv, 
                   NULL, NULL);
	
	gtk_init(NULL, NULL);
	glade_init();

	if (getuid () != 0) {
		/* PPPoA must have root access */
		t_warn(NULL, _("Insuficient privileges"), _("PPPoA dialer requires root privileges to run."));
		t_su_exec (argv[0], NULL);
		exit (0);
	}
	/* make sure we have sbin in the path */
	path = g_strdup (g_getenv ("PATH"));
	if (!path)
		path = g_strdup ("/usr/bin:/usr/local/bin");
	tmp_path = g_strconcat (path, ":/usr/sbin:/usr/local/sbin", NULL);
	g_free (path);
	g_setenv ("PATH", tmp_path, TRUE);
	g_free (tmp_path);
	
	glade_file = g_strdup(BR_DATADIR("/gnome-ppx/libpppd-gnome.glade"));
	m = ppp_manager_new (glade_file, "pppoa", "/apps/gnome-pppoa");
	d = PPP_DIALER (pppoa_dialer_new (m, BR_DATADIR ("/gnome-pppoa/addressbook.xml")));
	g_free (glade_file);
	
	ppp_manager_set_dialer (m, d);
	g_object_unref (d);
	if (ppp_manager_run (m))
		gtk_main();
	
	g_object_unref (m);
	
	return 0;

}
