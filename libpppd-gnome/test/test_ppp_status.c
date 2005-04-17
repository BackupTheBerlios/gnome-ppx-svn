#include <pppd_gnome.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/gnome-i18n.h>

void on_stopped (PppStatus *s, gpointer data)
{
	gtk_main_quit();
}

void on_about (PppStatus *s, gpointer data)
{
	GtkWidget *about_dlg = NULL;
	const char *authors[] = {
		"Tiago Cogumbreiro <cogumbreiro@sf.users.net>",
		"Milton Moura <mgcm@acm.org>",
		NULL
	};
	const char *translators[] = {NULL};
	about_dlg = GTK_WIDGET(gnome_about_new(
		"GNOME PPPoE",
		"1.0",
		"Copyright (C) 2004 Tiago Cogumbreiro\nCopyright (C) 2004 Milton Moura",
		_("PPPoE Dialer"),
		authors,
		translators,
		NULL,
		NULL));
	g_return_if_fail(about_dlg);
	gtk_widget_show_all(about_dlg);
}

int main(int argc, char **argv)
{
	PppStatus *s;
	GnomeProgram *app;
	app = gnome_program_init("gnome-pppoe", "1.0", LIBGNOMEUI_MODULE, argc, argv, 
                   NULL, NULL);
	
	gtk_init(NULL, NULL);
	glade_init();
	s = ppp_status_new("../glade/libpppd-gnome.glade", "pppoe", "/apps/gnome-pppoe/use_tray");
	g_signal_connect(s,"stopped", G_CALLBACK(on_stopped), NULL);
	g_signal_connect(s,"about", G_CALLBACK(on_about), NULL);
	ppp_status_attach(s);
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	g_thread_exit(NULL);
	return 0;
}
