#include <tlib/tlib.h>
#include <gtk/gtk.h>

int main () {
	TTrayIcon *tray;
	GtkWidget *w, *img;
	
	gtk_init (NULL, NULL);
	tray = (TTrayIcon *)t_tray_icon_new();
	img = gtk_image_new_from_stock(
		GTK_STOCK_NETWORK,
		GTK_ICON_SIZE_SMALL_TOOLBAR
	);
	gtk_widget_show(img);
	t_tray_icon_set_image(tray, GTK_IMAGE(img));
	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	t_tray_icon_set_window (tray, GTK_WINDOW(w));
	gtk_widget_show (GTK_WIDGET(tray));
	t_tray_icon_set_tooltip (tray, "tooltip");
	gtk_main();
	return 0;
}
