#include "tsu.h"
#include "tsu-gnomesu.h"
#include "tsu-gksu.h"
#include "tmacros.h"
#include <glib/gi18n.h>

G_LOCK_DEFINE_STATIC (startup);

GQuark
t_su_get_error_domain (void)
{
	static GQuark domain = 0;
	if (G_UNLIKELY (domain == 0)) {
		domain = g_quark_from_static_string ("t-module-error");
	}
	return domain;
}

static const TSuModule *
t_su_load_module (const TSuModule *mods[])
{
	gint i;
	GError *err = NULL;
	
	for (i = 0; mods[i]; i++) {
		
		if (t_module_load_multiple (mods[i]->modules, G_MODULE_BIND_LAZY, NULL, &err))
			return mods[i];
		
		T_ERR_WARN (err);
	}
	return NULL;
}

static const TSuModule *
t_su_get_default_module (void)
{
	static gboolean startup = FALSE;
	static const TSuModule *module;
	
	G_LOCK (startup);
	if (G_UNLIKELY (!startup)) {
		const TSuModule *mods[] = {
			t_gnomesu_module(),
			/*t_gksu_module(),*/
			NULL
		};
		module = t_su_load_module (mods);
		if (module) {
			/* make this module resident */
			t_module_foreach (module->modules, (TModuleFunc) g_module_make_resident, NULL);
		}
	}
	G_UNLOCK (startup);
	
	return module;
}

gboolean
t_su_exec (const gchar *command, GError **error)
{
	const TSuModule *mod;
	mod = t_su_get_default_module ();
	
	if (!mod) {
		if (error)
			*error = g_error_new (
				T_SU_ERROR,
				T_SU_ERROR_UNSUPPORTED,
				_("System does not support running applications as a different user.")
			);
		return FALSE;
	}
	return mod->su_exec (command, error);
}
