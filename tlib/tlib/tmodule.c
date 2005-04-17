#include "tmodule.h"

GQuark
t_module_get_error_domain (void)
{
	static GQuark domain = 0;
	if (G_UNLIKELY (domain == 0)) {
		domain = g_quark_from_static_string ("t-module-error");
	}
	return domain;
}

/*
 * Resests the interface's fields.
 * if @max is < 0 then all fields are reset.
 */
static void
t_module_iface_reset (TSymbol iface[], gint max)
{
	gint i;
	for (i = 0; iface[i].name; i++) {
		if (max >= 0 && i >= max)
			break;
		*(iface[i].symbol) = NULL;
	}
}

GModule *
t_module_new_from_iface (const gchar *name, TSymbol iface[], GModuleFlags flags, const gchar *ld_lib_path, GError **err)
{
	GModule *mod;
	gchar *path;
	gchar *dir;
	gchar **dir_list;
	gint i;
	
	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (iface, NULL);
	g_return_val_if_fail (err == NULL || *err == NULL, NULL);
	
	/* lookup module */
	if (!ld_lib_path) {
		dir = g_strconcat ("/lib:/usr/lib:/usr/local/lib", g_getenv ("LD_LIBRARY_PATH"), NULL);
	} else {
		dir = g_strdup (ld_lib_path);
	}
	
	/* lookup list of directories */
	dir_list = g_strsplit (dir, ":", -1);
	for (i = 0; dir_list[i]; i++) {
		/* skip empty strings */
		if (*(dir_list[i]) == '\0')
			continue;
		
		path = g_module_build_path (dir_list[i], name);
		mod = g_module_open (path, flags);
		g_free (path);
		if (mod)
			break;
	}
	g_strfreev (dir_list);
	
	if (!mod || !iface) {
		if (!mod && err) {
			*err = g_error_new (
				T_MODULE_ERROR,
				T_MODULE_ERROR_NOT_FOUND,
				"Could not load module %s.",
				name
			);
		}
		return mod;
	}
	
	for (i = 0; iface[i].name; i++) {
		if (!g_module_symbol (mod, iface[i].name, iface[i].symbol)) {
			if (err) {
				*err = g_error_new (
					T_MODULE_ERROR,
					T_MODULE_ERROR_SYMBOL_MISSING,
					"Symbol %s is missing.",
					iface[i].name
				);
			}
			t_module_iface_reset (iface, i);
			
			/* we don't need this module anymore */
			g_module_close (mod);
			return NULL;
		}
	}
	
	return mod;
}

gboolean
t_module_load_multiple (TModule mods[], GModuleFlags flags, const gchar *ld_lib_path, GError **err)
{
	gint i, j;
	g_return_val_if_fail (mods, FALSE);
	g_return_val_if_fail (err == NULL || *err == NULL, FALSE);
	for (i = 0; mods[i].name; i++) {
		mods[i].gmodule = t_module_new_from_iface (mods[i].name, mods[i].iface, flags, ld_lib_path, err);
		if (!mods[i].gmodule) {
			for (j = 0; j < i; j++) {
				/* reset this interface */
				t_module_iface_reset (mods[i].iface, -1);
				/* close the associated module */
				g_module_close (mods[i].gmodule);
				mods[i].gmodule = NULL;
			}
			return FALSE;
		}
	}
	return TRUE;
}

void
t_module_foreach (TModule mods[], TModuleFunc f, gpointer user_data)
{
	gint i;
	g_return_if_fail (mods);
	for (i = 0; mods[i].name; i++)
		f (mods[i].gmodule, user_data);
}
