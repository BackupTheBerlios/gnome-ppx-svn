#include <pppd_gnome.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>

int main(int argc, char **argv)
{
	PppDialer *d;
	PppStatus *s;
	GnomeProgram *app;
	app = gnome_program_init("gnome-pppoe", "1.0", LIBGNOMEUI_MODULE, argc, argv, 
                   NULL, NULL);
	
	gtk_init(NULL, NULL);
	glade_init();
	s = ppp_status_new("/tmp/libpppd-gnome.glade", "pppoe", "/apps/gnome-pppoe/use_tray");
	d = ppp_dialer_new(s, "/tmp/libpppd-gnome.glade", "/apps/gnome-pppoe");
	ppp_dialer_run(d);
	gtk_main();
	g_thread_exit(NULL);
	return 0;
}
