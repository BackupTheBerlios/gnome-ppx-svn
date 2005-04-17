/* tmacros.h
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

#ifndef __EGG_MACROS_H__
#define __EGG_MACROS_H__
G_BEGIN_DECLS

#ifndef G_DEFINE_INTERFACE_TYPE
/*
 * Convenience macro to ease the interface definition. Works like G_DEFINE_TYPE.
 */
#define G_DEFINE_INTERFACE_TYPE(TN,t_n)                     G_DEFINE_INTERFACE_TYPE_WITH_CODE (TN, t_n, {})
#define G_DEFINE_INTERFACE_TYPE_WITH_CODE(TypeName, type_name, CODE) \
static void     type_name##_init        (TypeName##Iface *iface); \
GType \
type_name##_get_type (void) \
{ \
  static GType g_define_interface_id = 0; \
  if (G_UNLIKELY (g_define_interface_id == 0)) \
    { \
      static const GTypeInfo g_define_type_info = { \
        sizeof (TypeName##Iface), \
        (GBaseInitFunc) NULL, \
        (GBaseFinalizeFunc) NULL, \
        (GClassInitFunc) type_name##_init, \
      }; \
      g_define_interface_id = g_type_register_static (G_TYPE_INTERFACE, #TypeName, &g_define_type_info, 0); \
    } \
    { CODE ;} \
  return g_define_interface_id; \
}
#endif /* G_DEFINE_INTERFACE_TYPE */

#define T_ERR_WARN(err) {if ((err)) {   \
	g_warning ("%s", (err)->message); \
	g_error_free ((err));             \
	(err) = NULL;                     \
}}

#define T_ERR_RETURN(err) {if ((err)) {  \
	g_warning ("%s", (err)->message); \
	g_error_free ((err));             \
	g_return_if_reached();            \
}}

#define T_ERR_RETURN_VAL(err,val) {if ((err)) { \
	g_warning ("%s", (err)->message);    \
	g_error_free ((err));                \
	g_return_val_if_reached(val);           \
}}

G_END_DECLS
#endif /* __EGG_MACROS_H__ */
