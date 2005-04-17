/*
 * TModule offers the ability to validate in a
 * typified manner the ability to load modules.
 */
#ifndef __T_MODULE_H_
#define __T_MODULE_H_

#include <gmodule.h>
#include <glib.h>

G_BEGIN_DECLS

#define T_MODULE_ERROR (t_module_get_error_domain ())

enum {
	T_MODULE_ERROR_NOT_FOUND,
	T_MODULE_ERROR_SYMBOL_MISSING
};

typedef struct _TSymbol TSymbol;
typedef struct _TModule TModule;
typedef void (*TModuleFunc) (GModule *module, gpointer data);

struct _TSymbol {
	gchar *name;
	gpointer *symbol;
};

struct _TModule {
	gchar *name;
	TSymbol *iface;
	GModule *gmodule;
};

GQuark t_module_get_error_domain (void);

/* @iface symbols' will be binded */
GModule *t_module_new_from_iface (const gchar *name, TSymbol iface[], GModuleFlags flags, const gchar *ld_lib_path, GError **err);

/* data will be written in the module. @iface's symbols will be binded.
 * @module will be filled too.
 */
gboolean t_module_load_multiple (TModule mods[], GModuleFlags flags, const gchar *ld_lib_path, GError **err);

/*
 * helper function that closes all modules in a static module array
 */
void t_module_foreach (TModule mods[], TModuleFunc f, gpointer user_data);

G_END_DECLS

#endif /* __T_MODULE_H_ */
