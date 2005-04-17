/* tglade.h
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
/*
   T::Glade is an utility module to enforce validation
   and simplify the usage of the Glade API.
   It recieves an array of widget names and their respective
   type, if the type is mismatched an exception is raised.
   It also recieves an array of signal names and the respective
   callback function and user data which the user wants to send.
 */

#ifndef __T_GLADE_H__
#define __T_GLADE_H__
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define T_GLADE_ERROR           (t_glade_get_error_domain())

enum {
	T_GLADE_ERROR_GLADE_FAILED,
	T_GLADE_ERROR_WIDGET_NOT_FOUND,
	T_GLADE_ERROR_TYPE_MISMATCHED,
};

typedef struct _TGladeSignal TGladeSignal;
typedef struct _TGladeInfo TGladeInfo;

struct _TGladeInfo {
	gchar *name;
	GType type;
};

struct _TGladeSignal {
	const gchar *name;
	GCallback callback;
	gpointer user_data;
};

GQuark t_glade_get_error_domain (void);

GHashTable *t_glade_parse (const gchar *file,
                          const gchar *domain,
                          const TGladeInfo widgets[],
                          const TGladeSignal signals[],
                          GError **err);

GHashTable *t_glade_parse_from_buffer (const gchar *buffer,
                          gint size,
                          const gchar *domain,
                          const TGladeInfo widgets[],
                          const TGladeSignal signals[],
                          GError **err);

G_END_DECLS
#endif /* __T_GLADE_H__ */
