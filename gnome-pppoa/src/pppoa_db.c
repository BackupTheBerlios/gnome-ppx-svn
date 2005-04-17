#include "pppoa_db.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <tlib/tlib.h>

typedef struct _PppoaDbPrivate PppoaDbPrivate;
#define PPPOA_DB_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PPPOA_TYPE_DB, PppoaDbPrivate))

static void pppoa_db_set_property(GObject * object, guint prop_id,
				    const GValue * value,
				    GParamSpec * pspec);
static void pppoa_db_finalize (GObject *object);

static gboolean pppoa_db_parse (PppoaDb *self, const gchar *filename);

G_DEFINE_TYPE (PppoaDb, pppoa_db, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_FILENAME,
};

struct _PppoaDbPrivate {
	GHashTable *db;
};

static void
pppoa_db_init (PppoaDb *self) {
	PPPOA_DB_GET_PRIVATE (self)->db = g_hash_table_new_full (
		g_str_hash,
		g_str_equal,
		g_free,
		(GDestroyNotify) g_hash_table_destroy
	);
}

static void pppoa_db_finalize (GObject *object)
{
	g_hash_table_destroy (PPPOA_DB_GET_PRIVATE (object)->db);
}

static void
pppoa_db_class_init (PppoaDbClass *klass)
{
	GObjectClass *g_klass;

	g_klass = G_OBJECT_CLASS (klass);
	g_klass->set_property = pppoa_db_set_property;
	g_klass->finalize = pppoa_db_finalize;
	g_type_class_add_private(g_klass, sizeof(PppoaDbPrivate));
	g_object_class_install_property (g_klass, PROP_FILENAME,
					g_param_spec_string("filename",
							    "The database filename",
							    "The database filename",
							    NULL,
							    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
pppoa_db_set_property (GObject * object, guint prop_id,
				    const GValue * value,
				    GParamSpec * pspec)
{
	if (prop_id == PROP_FILENAME)
		pppoa_db_parse (PPPOA_DB (object), g_value_get_string (value));
	else
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
}

PppoaDb *
pppoa_db_new (const gchar *filename)
{
	return PPPOA_DB (g_object_new (PPPOA_TYPE_DB, "filename", filename, NULL));
}

/**** actual code ****/

static void
pppoa_db_entry_free (PppoaDbEntry *entry)
{
	g_free (entry->isp);
	g_free (entry);
}

static gboolean
pppoa_db_parse (PppoaDb *self, const gchar *filename)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	xmlNodePtr country_iter, isp_iter, iter;
	GHashTable *db = PPPOA_DB_GET_PRIVATE (self)->db;
	GHashTable *entry_db;
	const gchar *country_code;
	const gchar *isp_name;
	gchar *content;
	PppoaDbEntry *entry;
	
	doc = xmlParseFile (filename);
	if (!doc)
		return FALSE;
	
	root_element = xmlDocGetRootElement (doc);
	
	for (country_iter = root_element->children; country_iter; country_iter = country_iter->next) {
		if (country_iter->type != XML_ELEMENT_NODE)
			continue;
		
		country_code = xmlGetProp (country_iter, "code");

		if (!country_code)
			continue;
			
		entry_db = (GHashTable *) g_hash_table_lookup (db, country_code);
		if (!entry_db) {
			/* create a new one and add it to the main db */
			entry_db = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) pppoa_db_entry_free);
			g_hash_table_insert (db, g_strdup (country_code), entry_db);
		}
		
		for (isp_iter = country_iter->children; isp_iter; isp_iter = isp_iter->next) {
			if (isp_iter->type != XML_ELEMENT_NODE)
				continue;
			
			isp_name = xmlGetProp (isp_iter, "name");
			if (!isp_name)
				continue;
			
			
			entry = g_new (PppoaDbEntry, 1);
			entry->isp = g_strdup (isp_name);
			
			for (iter = isp_iter->children; iter; iter = iter->next) {
				if (isp_iter->type != XML_ELEMENT_NODE)
					continue;
				
				if (!strcmp (iter->name, "vpi")) {
					content = xmlNodeGetContent (iter);
					entry->vpi = atoi (content);
					xmlFree (content);
				} else if (!strcmp (iter->name, "vci")) {
					content = xmlNodeGetContent (iter);
					entry->vci = atoi (content);
					xmlFree (content);
				}
			}
			/* add the entry to the hash */
			g_hash_table_insert (entry_db, g_strdup (isp_name), entry);
		}
	}
	
	xmlFreeDoc (doc);
	return TRUE;
}

GSList *
pppoa_db_get_entries (PppoaDb *self, const gchar *country_code)
{
	GSList *ret = NULL;
	
	GHashTable *isps;
	
	isps = g_hash_table_lookup (PPPOA_DB_GET_PRIVATE (self)->db, country_code);
	
	if (isps) {
		ret = t_hash_table_get_values (isps);
	}
	return ret;
}

GSList *
pppoa_db_get_countries (PppoaDb *self)
{
	return t_hash_table_get_keys (PPPOA_DB_GET_PRIVATE (self)->db);
}

PppoaDbEntry *
pppoa_db_get (PppoaDb *self, const gchar *country_code, const gchar *isp)
{
	PppoaDbEntry *ret = NULL;
	
	GHashTable *isps;
	
	isps = g_hash_table_lookup (PPPOA_DB_GET_PRIVATE (self)->db, country_code);
	
	if (isps) {
		ret = (PppoaDbEntry *) g_hash_table_lookup (isps, isp);
	}
	return ret;
}
