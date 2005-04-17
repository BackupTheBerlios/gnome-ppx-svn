/* tglade.c
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
#include "tglade.h"

#include <glade/glade.h>
#include <glib.h>

GQuark
t_glade_get_error_domain (void)
{
	static GQuark domain = 0;
	if (G_UNLIKELY (0 == domain)) {
		domain = g_quark_from_static_string ("t-glade-helper-error");
	}
	return domain;
}

static GHashTable *
t_glade_p (GladeXML *g, const TGladeInfo widgets[],
                        const TGladeSignal signals[],
                        GError **error)
{
	gint i;
	GtkWidget *w;
	GHashTable *ret;
	
	g_return_val_if_fail (widgets, NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);
	
	/* check if glade creation was successful */
	
	if (!g) {
		if (error)
			*error = g_error_new (T_GLADE_ERROR, T_GLADE_ERROR_GLADE_FAILED, "Error while parsing through libglade.");
	}
	
	ret = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	/* now try to load each widget */
	for (i = 0; widgets[i].name; i++) {
		/* check if widget exists */
		w = glade_xml_get_widget (g, widgets[i].name);
		if (!w) {
			if (error)
				*error = g_error_new (T_GLADE_ERROR, T_GLADE_ERROR_WIDGET_NOT_FOUND, "Widget '%s' was not found.", widgets[i].name);
			g_object_unref (g);
			g_hash_table_destroy  (ret);
			return NULL;
		}
		/* check if widget is of the correct type */
		if (!G_TYPE_CHECK_INSTANCE_TYPE (w, widgets[i].type)) {
			if (error)
				*error = g_error_new (
					T_GLADE_ERROR,
					T_GLADE_ERROR_TYPE_MISMATCHED,
					"Widget '%s' is of type '%s' instead of '%s'.",
					widgets[i].name,
					G_OBJECT_TYPE_NAME (w),
					g_type_name (widgets[i].type)
			);
			g_object_unref (g);
			g_hash_table_destroy  (ret);
			return NULL;
		}
		g_hash_table_insert (ret, g_strdup (widgets[i].name), w);
	}

	/* finally connect the signals */
	if (signals) {
		for (i = 0; signals[i].name; i++) {
			glade_xml_signal_connect_data (g, signals[i].name, signals[i].callback, signals[i].user_data);
		}
	}
	
	g_object_unref (g);
	return ret;
}

GHashTable *t_glade_parse (const gchar *file,
                          const gchar *domain,
                          const TGladeInfo widgets[],
                          const TGladeSignal signals[],
                          GError **err)
{
	GladeXML *g;
	g = glade_xml_new (file, widgets[0].name, domain);
	return t_glade_p (g, widgets, signals, err);
}

GHashTable *t_glade_parse_from_buffer (const gchar *buffer,
                          int size,
                          const gchar *domain,
                          const TGladeInfo widgets[],
                          const TGladeSignal signals[],
                          GError **err)
{
	GladeXML *g;
	g = glade_xml_new_from_buffer (buffer, size, widgets[0].name, domain);
	return t_glade_p (g, widgets, signals, err);
}
