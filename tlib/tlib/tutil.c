/* tutil.c
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

#include "tutil.h"
#include "tmacros.h"
#include <glib/gi18n.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

GQuark
t_util_get_error_domain (void)
{
	static GQuark domain = 0;
	if (!domain) {
		domain = g_quark_from_static_string ("t-util-error");
	}
	return domain;
}

static void
t_dialog_print (GtkWindow *parent, GtkMessageType type, const gchar *message, const gchar *description)
{
	GtkWidget *w;
	gchar *msg = g_strdup_printf ("<b><big>%s</big></b>\n\n%s", message, description);

	w = gtk_message_dialog_new_with_markup (
		parent,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		type,
		GTK_BUTTONS_CLOSE,
		msg
	);
	g_free (msg);
	gtk_dialog_run(GTK_DIALOG(w));
	gtk_widget_destroy(w);
}

void
t_error (GtkWindow *parent, const gchar *message, const gchar *description)
{
	t_dialog_print (parent, GTK_MESSAGE_ERROR, message, description);
}

void
t_warn (GtkWindow *parent, const gchar *message, const gchar *description)
{
	t_dialog_print (parent, GTK_MESSAGE_WARNING, message, description);
}

void
t_inform (GtkWindow *parent, const gchar *message, const gchar *description)
{
	t_dialog_print (parent, GTK_MESSAGE_INFO, message, description);
}

gboolean
t_update_program_in_gconf (const gchar *program, const gchar *key, GError **error)
{
	GConfClient *gconf;
	GError *err = NULL;
	gchar *bin, *bin_name;
	gboolean ret;
	
	g_return_val_if_fail (program, FALSE);
	g_return_val_if_fail (key, FALSE);
	g_return_val_if_fail (!error || !*error, FALSE);
	
	gconf = gconf_client_get_default ();
	
	bin = gconf_client_get_string(gconf, key, &err);
	if (err) {
		if (bin)
			g_free (bin);
		g_warning (
			"t_update_program_in_gconf: Error retrieving key %s: %s",
			key,
			err->message
		);
		g_error_free (err);
		err = NULL;
	}
	
	if (bin && g_path_is_absolute (bin) && 
               g_file_test(bin, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_EXECUTABLE)) {

		/* make sure program in gconf is the one we are checking */
		bin_name = g_path_get_basename (bin);
		ret = !strcmp (bin_name, program);
		g_free (bin_name);
		/* if the name is not the same, go on */
		
		if (ret) {
			g_object_unref (gconf);
			return TRUE;
		} else {
			/* since the gconf entry is not valid we'll try to remove it */
			gconf_client_unset (gconf, key, &err);
			if (err) {
				g_warning (
					"t_update_program_in_gconf: Could not unset key %s: %s",
					key,
					err->message
				);
				g_error_free (err);
				err = NULL;
			}
		}
	}
	
	if (bin)
		g_free (bin);
	
	bin = g_find_program_in_path (program);
	ret = TRUE;
	if (bin) {
		/* we found it, we must put it in gconf key */
		gconf_client_set_string (gconf, key, bin, error);
		if (*error) {
			ret = FALSE;
		}
	} else {
		*error = g_error_new (T_UTIL_ERROR, T_UTIL_ERROR_PROGRAM_NOT_FOUND, _("Program %s was not found."), program);
		ret = FALSE;
	}
	g_object_unref (gconf);
	return ret;
}

gboolean
t_update_program_in_gconf_gui (GtkWindow *parent, const gchar *program, const gchar *key)
{
	GError *error = NULL;
	gboolean ret;
	ret = t_update_program_in_gconf (program, key, &error);
	if (error) {
		gchar *title = g_strdup_printf ("Could not locate program <i>%s</i>", program);
		t_error (parent, title, error->message);
		g_free (title);
		g_error_free (error);
	}
	return ret;
}

static void
t_hash_foreach_keys (gpointer key, gpointer data, GSList **ret)
{
	*ret = g_slist_append (*ret, key);
}

GSList *
t_hash_table_get_keys (GHashTable *hash_table)
{
	GSList *ret = NULL;
	g_hash_table_foreach (hash_table, (GHFunc) t_hash_foreach_keys, &ret);
	return ret;
}

static void
t_hash_foreach_values (gpointer key, gpointer data, GSList **ret)
{
	*ret = g_slist_append (*ret, data);
}

GSList *
t_hash_table_get_values (GHashTable *hash_table)
{
	GSList *ret = NULL;
	g_hash_table_foreach (hash_table, (GHFunc) t_hash_foreach_values, &ret);
	return ret;
}

gboolean
t_tree_model_find (GtkTreeModel *model, int col, GValue *value, GtkTreeIter *iter, TCompareFunc cmp)
{
	gboolean has_next;
	GValue iter_value = {0,};
	gboolean equal_vals;
	GError *error = NULL;
	g_return_val_if_fail (GTK_IS_TREE_MODEL (model), FALSE);
	g_return_val_if_fail (iter, FALSE);
	g_return_val_if_fail (cmp, FALSE);
	
	has_next = gtk_tree_model_get_iter_first (model, iter);
	
	for (; has_next; has_next = gtk_tree_model_iter_next (model, iter)) {
		gtk_tree_model_get_value (model, iter, col, &iter_value);
		equal_vals = !cmp (&iter_value, value, &error);
		g_value_unset (&iter_value);
		if (error) {
			g_error_free (error);
			error = NULL;
		} else if (equal_vals) {
			/* no error occurred and values are equal */
			return TRUE;
		}
	}
	return FALSE;
}

gint
t_compare_string (GValue *val1, GValue *val2, GError **error)
{
	g_return_val_if_fail (error == NULL || *error == NULL, 0);
	g_return_val_if_fail (val1, 0);
	g_return_val_if_fail (val2, 0);
	
	if (!G_VALUE_HOLDS_STRING (val1) || !G_VALUE_HOLDS_STRING (val2)) {
		*error = g_error_new (
			T_UTIL_ERROR,
			T_UTIL_ERROR_TYPE_MISMATCH,
			"Value 1 '%s' and value 2 '%s' should be strings.",
			G_VALUE_TYPE_NAME (val1),
			G_VALUE_TYPE_NAME (val2)
		);
		return 0;
	}
	
	return strcmp (g_value_get_string (val1), g_value_get_string (val2));
}

gboolean
t_combo_box_set_active_string (GtkComboBox *cbox, gint col, const gchar *str)
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_combo_box_get_model (cbox);
	gboolean found;
	GValue val = {0, };
	
	g_value_init (&val, G_TYPE_STRING);
	g_value_set_string (&val, str);
	
	found = t_tree_model_find (model, col, &val, &iter, t_compare_string);
	
	if (found) {
		/* activate iterator */
		gtk_combo_box_set_active_iter (cbox, &iter);
	}
	
	g_value_unset (&val);
	return found;
}

static gboolean
t_text_tail_print (GIOChannel *source, GIOCondition condition, gpointer data)
{
	GtkTextView *text_view;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	gchar *line;
	gsize len;
	GError *err = NULL;
	
	if (G_IO_STATUS_NORMAL != g_io_channel_read_line (source, &line, &len, NULL, &err)) {
		if (len > 0)
			g_free (line);
		T_ERR_WARN (err);
		return FALSE;
	}
	T_ERR_WARN (err);
	
	if (!len) {
		g_warning ("Empty line");
		return TRUE;
	}
	
	text_view = GTK_TEXT_VIEW (data);
	
	/* append text */
	buffer = gtk_text_view_get_buffer (text_view);
	gtk_text_buffer_get_end_iter (buffer, &iter);
	gtk_text_buffer_insert (buffer, &iter, line, -1);
	g_free (line);
	
	/* move carret to the end */
	gtk_text_buffer_get_end_iter (buffer, &iter);
	gtk_text_buffer_move_mark_by_name (buffer, "insert", &iter);

	/* scroll to carret */
	gtk_text_view_scroll_to_mark (
		text_view,
	    gtk_text_buffer_get_mark (buffer, "insert"),
		0.0,
		FALSE,
		0.0,
		0.0
	);
	return TRUE;
}

static gboolean
t_fd_to_string_print (GIOChannel *source, GIOCondition condition, gpointer data)
{
	gchar *line;
	gsize len;
	GError *err = NULL;
	GSList **buffer = (GSList **) data;
	
	if (G_IO_STATUS_NORMAL != g_io_channel_read_line (source, &line, &len, NULL, &err)) {
		if (len > 0)
			g_free (line);
		T_ERR_WARN (err);
		return FALSE;
	}
	T_ERR_WARN (err);
	
	if (!len) {
		g_warning ("Empty line");
		return TRUE;
	}
	
	*buffer = g_slist_append (*buffer, line);
	return TRUE;
}

void
t_fd_to_string (GSList **buffer, int fd)
{
	GIOChannel *channel;
	g_return_if_fail (buffer != NULL && *buffer == NULL);
	
	channel = g_io_channel_unix_new (fd);
	g_io_add_watch (channel, (G_IO_IN | G_IO_HUP | G_IO_ERR), t_fd_to_string_print, buffer);
	g_io_channel_unref (channel);
}

void
t_text_view_tail (GtkTextView *w, int fd)
{
	GIOChannel *channel;
	g_return_if_fail (fd < 0);
	g_return_if_fail (GTK_IS_TEXT_VIEW (w));
	
	channel = g_io_channel_unix_new (fd);
	g_io_add_watch (channel, (G_IO_IN | G_IO_HUP | G_IO_ERR), t_text_tail_print, w);
	g_io_channel_unref (channel);
}

gboolean
t_has_suid (const gchar *filename)
{
	struct stat s;
	if (stat (filename, &s))
		return FALSE;
	return (s.st_mode & S_ISUID) == S_ISUID;
}
