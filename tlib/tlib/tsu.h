#ifndef __T_SU_H_
#define __T_SU_H_

#include <glib.h>
#include "tmodule.h"

G_BEGIN_DECLS

#define T_SU_ERROR (t_su_get_error_domain())

enum {
	/* tsu is not available */
	T_SU_ERROR_UNSUPPORTED,
	/* command was not found */
	T_SU_ERROR_COMMAND_NOT_FOUND,
	/* password failed */
	T_SU_ERROR_PASSWORD_FAILED,
	/* unknow error returned by backend */
	T_SU_ERROR_UNKNOWN
};

typedef struct _TSuModule TSuModule;

struct _TSuModule {
	/* a su module can be constituted by multiple mods, example: gksu */
	TModule *modules;
	/* you cannot throw T_SU_ERROR_UNSUPPORTED errors */
	gboolean (*su_exec) (const gchar *cmd, GError **error);
};

GQuark t_su_get_error_domain (void);

/* throws errors from T_SU_ERROR domain */
gboolean t_su_exec (const gchar *command, GError **error);

G_END_DECLS
#endif /* __T_SU_H_ */
