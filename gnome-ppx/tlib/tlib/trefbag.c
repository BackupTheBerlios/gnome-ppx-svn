/* trefbag.c
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

#include "trefbag.h"

struct _TRefbag {
	TRefbagSetFunc set;
	GSList *list;
	gint32 ref_count;
};

static void t_refbag_set_gobject (gpointer *ptr, gpointer value)
{
	if (*ptr)
		g_object_unref(*ptr);
	if (value)
		g_object_ref(value);
	*ptr = value;
}

static void t_refbag_set_string (gpointer *ptr, gpointer value)
{
	if (*ptr)
		g_free(*ptr);
	if (value)
		*ptr = g_strdup((const gchar *) value);
	else
		*ptr = NULL;
}

TRefbag * t_refbag_new_gobject (void)
{
	return t_refbag_new (t_refbag_set_gobject);
}

TRefbag * t_refbag_new_string (void)
{
	return t_refbag_new (t_refbag_set_string);
}

TRefbag * t_refbag_new (TRefbagSetFunc set)
{
	TRefbag *ret;
	
	ret = g_new (TRefbag, 1);
	ret->list = NULL;
	ret->set = set;
	ret->ref_count = 1;
	
	return ret;
}

void t_refbag_add_full (TRefbag *self, gpointer *ref, gpointer init_value)
{
	g_return_if_fail(self);
	self->list = g_slist_append(self->list, ref);
	*ref = init_value;
}

void t_refbag_set (TRefbag *self, gpointer *ref, gpointer value)
{
	g_return_if_fail(self);
	self->set(ref, value);
}

void t_refbag_set_refed (TRefbag *bag, gpointer *ref, gpointer value)
{
	t_refbag_set(bag, ref, NULL);
	*ref = value;
}

void t_refbag_ref (TRefbag *bag)
{
	bag->ref_count++;
}

void t_refbag_unref (TRefbag *bag)
{
	bag->ref_count--;
	if (bag->ref_count < 0) {
		/* now we must iterate over each pointer to the ref */
		g_slist_foreach (bag->list, (GFunc)bag->set, NULL);
		g_slist_free(bag->list);
		g_free(bag);
	}
}
