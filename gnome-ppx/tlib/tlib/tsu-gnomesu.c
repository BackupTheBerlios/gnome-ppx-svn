
#include "tsu-gnomesu.h"
#include <glib/gi18n.h>

static gboolean (*gnomesu_exec) (const gchar *commandline);

static TSymbol gnomesu_iface[] = {
	{"gnomesu_exec", (gpointer *) &gnomesu_exec},
	{NULL}
};

static TModule mods[] = {
	{"gnomesu", gnomesu_iface, NULL},
	{NULL}
};

static gboolean
t_gnomesu_exec (const gchar *cmd, GError **error)
{
	gboolean ret = gnomesu_exec (cmd);
	if (!ret && error) {
		*error = g_error_new (T_SU_ERROR, T_SU_ERROR_UNKNOWN, _("Could not run command '%s' as a different user"), cmd);
	}
	return ret;
}

const TSuModule *
t_gnomesu_module (void)
{
	static TSuModule su_mod = {mods, t_gnomesu_exec};
	return &su_mod;
}
