/* trefbag.h
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
 * A Reference Bag is a utility data structure for the boring task
 * of referencing and unreferencing other data types. It comes bundled
 * with two helper constructor for the two most commonly kept references
 * strings and gobjects. 
 * Keep in mind that when the RefBag is released
 * it sets every reference to NULL, therefore releasing all associated
 * data.
 */
#ifndef __T_REFBAG_H__
#define __T_REFBAG_H__
#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _TRefbag TRefbag;
typedef void (*TRefbagSetFunc) (gpointer *ref, gpointer value);

TRefbag * t_refbag_new (TRefbagSetFunc set);
TRefbag * t_refbag_new_gobject (void);
TRefbag * t_refbag_new_string (void);
/* a macro which inits the value to NULL */
#define t_refbag_add(bag,ref) t_refbag_add_full((bag), (gpointer *)(ref), NULL)
/* adds a new reference pointer and initializes it with a value,
 * note that this value must already be with reference counting set
 * to one, because it will only be initialized with the init_value
 */
void t_refbag_add_full (TRefbag *bag, gpointer *ref, gpointer init_value);

void t_refbag_set(TRefbag *bag, gpointer *ref, gpointer value);
/* a simple helper function which does not do anything to the new value,
 * yet it unrefs the old one. It's the same as:
 * ref_bag_set(bag, ref, NULL);
 * *ref = value;
 */
void t_refbag_set_refed (TRefbag *bag, gpointer *ref, gpointer value);
void t_refbag_ref (TRefbag *bag);
void t_refbag_unref (TRefbag *bag);

G_END_DECLS
#endif /* __T_REFBAG_H__ */
