/* ttrayicon.c
 * Copyright (C) 2004 Tiago Cogumbreiro <cogumbreiro@users.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <glib/gi18n.h>
#include "ttrayicon.h"

#define BTI_LEFT_CLICK  1
#define BTI_MIDDLE_CLICK 2
#define BTI_RIGHT_CLICK 3


static GObjectClass *parent_class = NULL;

/* Private prototypes */
static void t_tray_icon_class_init(TTrayIconClass * klass);
static void t_tray_icon_init(TTrayIcon * object);
static void t_tray_icon_finalize(GObject * object);
static void t_tray_icon_set_property(GObject * object, guint prop_id,
					 const GValue * value,
					 GParamSpec * pspec);
static void t_tray_icon_get_property(GObject * object, guint prop_id,
					 GValue * value,
					 GParamSpec * pspec);
static gboolean t_tray_icon_clicked (GtkWidget *ebox, GdkEventButton *evt, 
					 TTrayIconPrivate *p);
#if GLIB_CHECK_VERSION (2,4,0)
#define T_TRAY_ICON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), T_TYPE_TRAY_ICON, TTrayIconPrivate))
#else
#define T_TRAY_ICON_GET_PRIVATE(obj) (T_TRAY_ICON(obj)->private_data)
#endif

struct _TTrayIconPrivate {
	GtkTooltips *tooltips;
	GtkWidget *window;
	GtkContainer *ebox;
	GtkMenu *menu;
};

enum {
	PROP_0,
	PROP_TOOLTIP,
	PROP_WINDOW,
	PROP_IMAGE,
	PROP_MENU,
	PROP_1
};

GType t_tray_icon_get_gobject_type()
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof(TTrayIconClass),
			NULL,	/* base init */
			NULL,	/* base finalize */
			(GClassInitFunc) t_tray_icon_class_init,
			NULL,	/* class finalize */
			NULL,	/* class data */
			sizeof(TTrayIcon),
			0,	/* number of pre-allocs */
			(GInstanceInitFunc) t_tray_icon_init,
		};
		object_type =
		    g_type_register_static(EGG_TYPE_TRAY_ICON,
					   "TTrayIcon", &object_info,
					   0);
	}
	return object_type;
}

static void t_tray_icon_class_init(TTrayIconClass * klass)
{
	GObjectClass *gobject_class;

	parent_class = g_type_class_peek_parent(klass);
	gobject_class = G_OBJECT_CLASS(klass);
#if GLIB_CHECK_VERSION(2,4,0)
	/* Must have GLib 2.4 in order to register private data */
	g_type_class_add_private(gobject_class,
				 sizeof(TTrayIconPrivate));
#endif
	gobject_class->finalize = t_tray_icon_finalize;
	gobject_class->set_property = t_tray_icon_set_property;
	gobject_class->get_property = t_tray_icon_get_property;
	g_object_class_install_property(gobject_class, PROP_TOOLTIP,
					g_param_spec_string("tooltip",
							    _("Tray's tooltip"),
							    _("Sets the tray's tooltip."),
							    NULL,
							    G_PARAM_WRITABLE));
	g_object_class_install_property(gobject_class, PROP_WINDOW,
					g_param_spec_object("window",
							    _("Default window"),
							    _("The default window which will be shown/hidden when user left clicks on tray."),
							    GTK_TYPE_WINDOW,
							    G_PARAM_READWRITE));
	g_object_class_install_property(gobject_class, PROP_IMAGE,
					g_param_spec_object("image",
							    _("Tray image"),
							    _("The image that will be shown in the tray."),
							    GTK_TYPE_IMAGE,
							    G_PARAM_READWRITE));
	g_object_class_install_property(gobject_class, PROP_MENU,
					g_param_spec_object("menu",
							    _("Tray menu"),
							    _("The popup menu that will be shown when user right clicks on tray."),
							    GTK_TYPE_IMAGE,
							    G_PARAM_READWRITE));
}

GtkWidget *t_tray_icon_new()
{
	return GTK_WIDGET(g_object_new(T_TYPE_TRAY_ICON, NULL));
}

static void t_tray_icon_init(TTrayIcon * object)
{
	TTrayIconPrivate *p;

#if !GLIB_CHECK_VERSION (2,4,0)
	/* we must allocate our private data */
	object->private_data = g_new(TTrayIconPrivate, 1);
#endif
	p = T_TRAY_ICON_GET_PRIVATE(object);
	p->tooltips = gtk_tooltips_new();
	p->window = NULL;
	p->menu = NULL;

	p->ebox = GTK_CONTAINER(gtk_event_box_new());
	gtk_widget_show (GTK_WIDGET(p->ebox));
	g_signal_connect(p->ebox, "button-press-event", G_CALLBACK(t_tray_icon_clicked), p);
	gtk_container_add(GTK_CONTAINER(object), GTK_WIDGET(p->ebox));
}

static void t_tray_icon_finalize(GObject * object)
{
	TTrayIconPrivate *private_data =
	    T_TRAY_ICON_GET_PRIVATE(object);
	g_object_unref(private_data->tooltips);
	if (private_data->window) {
		g_object_unref(private_data->window);
	}
#if !GLIB_CHECK_VERSION (2,4,0)
	/* we have to free our manually created data */
	g_free(private_data);
#endif				/* !MIN_GLIB_2_4 */
}

static void t_tray_icon_set_property(GObject * object, guint prop_id,
					 const GValue * value,
					 GParamSpec * pspec)
{
	TTrayIcon *self;

	self = T_TRAY_ICON(object);
	switch (prop_id) {
	case PROP_TOOLTIP:
		t_tray_icon_set_tooltip(self,
					    g_value_get_string(value));
		break;
	case PROP_WINDOW:
		t_tray_icon_set_window(self,
					   g_value_get_object(value));
		break;
	case PROP_IMAGE:
		t_tray_icon_set_image(self, g_value_get_object(value));
		break;
	case PROP_MENU:
		t_tray_icon_set_menu (self, g_value_get_object(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void t_tray_icon_get_property(GObject * object, guint prop_id,
					 GValue * value,
					 GParamSpec * pspec)
{
	TTrayIcon *self;

	self = T_TRAY_ICON(object);
	switch (prop_id) {
	case PROP_WINDOW:
		g_value_set_object(value,
				   t_tray_icon_get_window(self));
		break;
	case PROP_IMAGE:
		g_value_set_object(value, t_tray_icon_get_image(self));
		break;
	case PROP_MENU:
		g_value_set_object(value, t_tray_icon_get_menu(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

void t_tray_icon_set_tooltip(TTrayIcon * self,
				 const gchar * tooltip)
{
	gtk_tooltips_set_tip (
		T_TRAY_ICON_GET_PRIVATE(self)->tooltips,
		GTK_WIDGET(self),
		tooltip,
		NULL
	);
}

void t_tray_icon_set_window(TTrayIcon * self, GtkWindow * window)
{
	TTrayIconPrivate *private_data =
	    T_TRAY_ICON_GET_PRIVATE(self);
	g_return_if_fail (T_IS_TRAY_ICON (self));
	g_return_if_fail (GTK_IS_WINDOW (window));
	if (window)
		g_object_ref(window);
	if (private_data->window)
		g_object_unref(private_data->window);
	private_data->window = GTK_WIDGET(window);
}

GtkWindow *t_tray_icon_get_window(TTrayIcon * self)
{
	return GTK_WINDOW(T_TRAY_ICON_GET_PRIVATE(self)->window);
}

void t_tray_icon_set_image(TTrayIcon * self, GtkImage * image)
{
	TTrayIconPrivate *p = T_TRAY_ICON_GET_PRIVATE(self);
	GtkWidget *child;

	g_return_if_fail (T_IS_TRAY_ICON (self));
	g_return_if_fail (GTK_IS_IMAGE (image));
	
	child = gtk_bin_get_child(GTK_BIN(p->ebox));
	if (child != NULL)
		gtk_container_remove(p->ebox, child);
	
	gtk_container_add(p->ebox, GTK_WIDGET(image));
}

GtkImage *t_tray_icon_get_image(TTrayIcon * self)
{
	TTrayIconPrivate *p = T_TRAY_ICON_GET_PRIVATE(self);
	return GTK_IMAGE(gtk_bin_get_child(GTK_BIN(p->ebox)));;
}

void t_tray_icon_set_menu(TTrayIcon * self, GtkMenu * menu)
{
	TTrayIconPrivate *p = T_TRAY_ICON_GET_PRIVATE(self);
	g_return_if_fail (T_IS_TRAY_ICON (self));
	g_return_if_fail (GTK_IS_MENU (menu));
	
	if (menu)
		g_object_ref(menu);
	if (p->menu)
		g_object_unref(p->menu);
	p->menu = menu;
}

GtkMenu *t_tray_icon_get_menu(TTrayIcon * self)
{
	return T_TRAY_ICON_GET_PRIVATE(self)->menu;
}

static gboolean t_tray_icon_clicked (GtkWidget *ebox, GdkEventButton *evt, TTrayIconPrivate *p)
{
	if (evt->type != GDK_BUTTON_PRESS)
		return FALSE;
	
	switch (evt->button) {
		case BTI_LEFT_CLICK:
			if (p->window) {
				if (GTK_WIDGET_VISIBLE(p->window))
					gtk_widget_hide(p->window);
				else
					gtk_widget_show(p->window);
			}
			break;
			
		case BTI_RIGHT_CLICK:
			if (p->menu)
				gtk_menu_popup (
					p->menu,
					NULL,
					NULL,
					NULL,
					NULL,
					evt->button,
					evt->time
				);
			break;
	}
	return FALSE;
}
