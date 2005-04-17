#ifndef __T_COUNTRIES_H__
#define __T_COUNTRIES_H__

#include <glib.h>

G_BEGIN_DECLS
/* Converts an ISO 3166 code to country name. This name is translatable.
 */
const gchar *t_code_to_country (const gchar *code);

G_END_DECLS
#endif /* __T_COUNTRIES_H__ */
