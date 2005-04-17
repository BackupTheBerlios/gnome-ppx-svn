/* tutil.h
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

#ifndef __T_UTIL_H__
#define __T_UTIL_H__

#include <gtk/gtk.h>
#include <stdarg.h>
#include <gconf/gconf-client.h>

G_BEGIN_DECLS

/* error is returned if types are not of the same type or cannot be compared 
 * throws T_UTIL_ERROR_TYPE_MISMATCH when types cannot be compared
 */
typedef gint (*TCompareFunc) (GValue *val1, GValue *val2, GError **error);

#define T_UTIL_ERROR (t_util_get_error_domain())
enum {
	T_UTIL_ERROR_PROGRAM_NOT_FOUND,
	T_UTIL_ERROR_TYPE_MISMATCH,
};

GQuark t_util_get_error_domain (void);
void t_error (GtkWindow *parent, const gchar *message, const gchar *description);
void t_warn (GtkWindow *parent, const gchar *message, const gchar *description);
void t_inform (GtkWindow *parent, const gchar *message, const gchar *description);
gboolean t_update_program_in_gconf (const gchar *program, const gchar *key, GError **error);
gboolean t_update_program_in_gconf_gui (GtkWindow *parent, const gchar *program, const gchar *key);
/* returns a list of the existing keys/values of an hash table, the list must be released
 * with g_slist_free 
 */
GSList *t_hash_table_get_keys (GHashTable *table);
GSList *t_hash_table_get_values (GHashTable *hash_table);

/* compares two strings */
gint t_compare_string (GValue *val1, GValue *val2, GError **error);
/* finds a value in a list store 
 */
gboolean t_tree_model_find (GtkTreeModel *model, gint col, GValue *value, GtkTreeIter *iter, TCompareFunc cmp);
/* selects the first element in a combo box which has the same value as @value.
 * You can select the @column of the GtkTreeModel used to construct your combo box
 * which contains only strings. If your combo box was created with a gtk_combo_box_new_text
 * then you can use the utility t_combo_box_set_active_string_default which defaults the column
 * to number 0.
 */
gboolean t_combo_box_set_active_string (GtkComboBox *combo_box, gint column, const gchar *value);
#define t_combo_box_set_active_string_default(combo_box,value) (t_combo_box_select (combo_box, 0, value))

/*
 * Appends the content of an fd to a text view widget asynchronously
 */
void t_text_view_tail (GtkTextView *w, int fd);

/*
 * Data is filled into buffer asynchronously
 */
void t_fd_to_string (GSList **buffer, int fd);

/*
 * Checks if the file has the suid bit set on
 */
gboolean t_has_suid (const gchar *filename);

G_END_DECLS

#endif /* __T_UTIL_H__ */
