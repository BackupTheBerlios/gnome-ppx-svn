#include <gtk/gtk.h>
#include <bacon-cd-selection.h>

static GtkWidget *bcs_menu = NULL;

static void
on_file_image_checkbutton_toggled (GtkToggleButton *widget,
                                   gpointer         data)
{
        gboolean value = gtk_toggle_button_get_active (widget);
        g_message ("Setting file image value to %s", value ? "TRUE":"FALSE");
        g_object_set (bcs_menu, "file-image", value, NULL);
}

static void
on_show_recorders_only_checkbutton_toggled (GtkToggleButton *widget,
                                            gpointer         data)
{
        gboolean value = gtk_toggle_button_get_active (widget);
        g_message ("Setting show recorders value to %s", value ? "TRUE":"FALSE");
        g_object_set (bcs_menu, "show-recorders-only", value, NULL);
}

int
main (int argc, char **argv)
{
        GtkWidget *dialog;
        GtkWidget *widget;
	GtkWidget *vbox;

	gtk_init (&argc, &argv);

	dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (dialog), vbox);

	bcs_menu = bacon_cd_selection_new ();
	gtk_box_pack_start_defaults (GTK_BOX (vbox), bcs_menu);

	widget = gtk_check_button_new_with_label ("Show only recorders");
        g_signal_connect (widget, "toggled",
			G_CALLBACK (on_show_recorders_only_checkbutton_toggled), NULL);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), widget);
	gtk_widget_set_sensitive (widget, g_object_class_find_property (G_OBJECT_GET_CLASS (bcs_menu), "show-recorders-only") != NULL);

	widget = gtk_check_button_new_with_label ("Show file image");
        g_signal_connect (widget, "toggled",
                          G_CALLBACK (on_file_image_checkbutton_toggled), NULL);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), widget);

	gtk_widget_show_all (dialog);
	gtk_main ();

        return 0;
}


