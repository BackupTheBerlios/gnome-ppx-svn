#ifndef __PPPOA_DB_H__
#define __PPPOA_DB_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PPPOA_TYPE_DB            (pppoa_db_get_type ())
#define PPPOA_DB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PPPOA_TYPE_DB, PppoaDb))
#define PPPOA_DB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST    ((klass), PPPOA_TYPE_DB, PppoaDbClass))
#define PPPOA_IS_DB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPPOA_TYPE_DB))
#define PPPOA_IS_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE    ((klass), PPPOA_TYPE_DB))
#define PPPOA_DB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS  ((obj), PPPOA_TYPE_DB, PppoaDbClass))

typedef struct _PppoaDb PppoaDb;
typedef struct _PppoaDbClass PppoaDbClass;
typedef struct _PppoaDbEntry PppoaDbEntry;

struct _PppoaDbClass {
	GObjectClass parent;
};

struct _PppoaDb {
	GObject parent;
};

struct _PppoaDbEntry {
	gchar *isp;
	int vpi;
	int vci;
};

PppoaDb* pppoa_db_new (const gchar *filename);

/* returns a list of const ghcar * which are the country codes */
GSList *pppoa_db_get_countries (PppoaDb *self);
/* returns a list of const PppoaDbEntry * which represents an ISP */
GSList *pppoa_db_get_entries (PppoaDb *self, const gchar *country_code);
/* get a single entry */
PppoaDbEntry *pppoa_db_get (PppoaDb *self, const gchar *country_code, const gchar *isp);

GType pppoa_db_get_type (void);

G_END_DECLS
#endif /* __PPPOA_DB_H__ */
