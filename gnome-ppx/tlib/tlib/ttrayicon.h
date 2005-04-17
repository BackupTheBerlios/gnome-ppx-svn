/* ttrayicon.h
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
 
#ifndef __T_TRAY_ICON_H__
#define __T_TRAY_ICON_H__
#include <glib.h>
#include <glib-object.h>
#include "eggtrayicon.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS
	
#define T_TYPE_TRAY_ICON (t_tray_icon_get_gobject_type())
#define T_TRAY_ICON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), T_TYPE_TRAY_ICON, TTrayIcon))
#define T_TRAY_ICON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), T_TYPE_TRAY_ICON, TTrayIconClass))
#define T_IS_TRAY_ICON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), T_TYPE_TRAY_ICON))
#define T_IS_TRAY_ICON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),T_TYPE_TRAY_ICON))
#define T_TRAY_ICON_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), T_TYPE_TRAY_ICON, TTrayIconClass))
/* Object/Class definition */
typedef struct _TTrayIcon TTrayIcon;
typedef struct _TTrayIconClass TTrayIconClass;

/* Object structure */
struct _TTrayIconClass {
	EggTrayIconClass parent_class;
};

typedef struct _TTrayIconPrivate TTrayIconPrivate;
struct _TTrayIcon {
	EggTrayIcon parent;

#if !GLIB_CHECK_VERSION (2,4,0)
	/* private data */
	TTrayIconPrivate *private_data;
#endif /* !MIN_GLIB_2_4 */
};

GType t_tray_icon_get_gobject_type ();
GtkWidget* t_tray_icon_new ();
void t_tray_icon_set_tooltip (TTrayIcon* self, const gchar* tooltip);
void t_tray_icon_set_window (TTrayIcon* self, GtkWindow* window);
GtkWindow* t_tray_icon_get_window (TTrayIcon* self);
void t_tray_icon_set_image (TTrayIcon* self, GtkImage* image);
GtkImage* t_tray_icon_get_image (TTrayIcon* self);
void t_tray_icon_set_menu (TTrayIcon * self, GtkMenu * menu);
GtkMenu *t_tray_icon_get_menu (TTrayIcon * self);

G_END_DECLS

#endif /* __T_TRAY_ICON_H__ */
