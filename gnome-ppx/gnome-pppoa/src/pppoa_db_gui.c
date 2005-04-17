#include "pppoa_db_gui.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <tlib/tlib.h>
#include <string.h>
#include <pppd_gnome.h>

typedef struct _PppoaDbGuiPrivate PppoaDbGuiPrivate;
#define PPPOA_DB_GUI_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PPPOA_TYPE_DB_GUI, PppoaDbGuiPrivate))

static void pppoa_db_gui_set_property(GObject * object, guint prop_id,
				    const GValue * value,
				    GParamSpec * pspec);
static void pppoa_db_gui_finalize (GObject * object);

static void pppoa_db_gui_on_isp_changed (GtkComboBox *cbox, PppoaDbGui *db_gui);
static void pppoa_db_gui_on_country_changed (GtkComboBox *cbox, PppoaDbGui *db_gui);

static void pppoa_db_gui_set_db (PppoaDbGui *self, PppoaDb *db);
static void pppoa_db_gui_populate_countries (PppoaDbGui *self, GtkTreeStore *store);
static gboolean pppoa_db_gui_is_country_visible (GtkTreeModel *model, GtkTreeIter *iter, PppoaDbGui *self);
static gboolean pppoa_db_gui_is_isp_visible (GtkTreeModel *model, GtkTreeIter *iter, PppoaDbGui *self);

G_DEFINE_TYPE (PppoaDbGui, pppoa_db_gui, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DB,
};

enum {
	/* firs column has the label that should be displayed */
	COL_LABEL = 0,
	/* the index of associated coutry, if it is a country then it has a -1 */
	COL_COUNTRY_INDEX,
	/* second column holds the county code, where appropriate */
	COL_COUNTRY_CODE,
	/* third column holds the PppoaDbEntry, where appropriate */
	COL_ISP_ENTRY,
};

enum {
	SIG_CHANGED,
	SIG_LAST
};

struct _PppoaDbGuiPrivate {
	PppoaDb *db;
	PppoaDbEntry *entry;
	const gchar *code;
};

static guint signals[SIG_LAST] = {0};

static void
pppoa_db_gui_class_init (PppoaDbGuiClass *klass)
{
	GObjectClass *g_klass;

	g_klass = G_OBJECT_CLASS (klass);
	g_klass->set_property = pppoa_db_gui_set_property;
	g_klass->finalize = pppoa_db_gui_finalize;
	g_type_class_add_private(g_klass, sizeof(PppoaDbGuiPrivate));
	g_object_class_install_property (g_klass, PROP_DB,
					g_param_spec_object("db",
							    "The database object",
							    "The database object",
							    PPPOA_TYPE_DB,
							    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	signals[SIG_CHANGED] = g_signal_new(
		"changed",
		G_OBJECT_CLASS_TYPE(g_klass),
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET (PppoaDbGuiClass, values_changed),
		NULL, NULL,
		ppp_marshallers_VOID__VOID,
		G_TYPE_NONE,
		0
	);
}

static void
pppoa_db_gui_init (PppoaDbGui *self) {
	PppoaDbGuiPrivate *p;
	p = PPPOA_DB_GUI_GET_PRIVATE (self);
	p->entry = NULL;
	p->code = NULL;
}

static void pppoa_db_gui_finalize (GObject * object)
{
	g_object_unref (PPPOA_DB_GUI_GET_PRIVATE (object)->db);
}

static void
pppoa_db_gui_set_property (GObject * object, guint prop_id,
				    const GValue * value,
				    GParamSpec * pspec)
{
	if (prop_id == PROP_DB)
		pppoa_db_gui_set_db (PPPOA_DB_GUI (object), g_value_get_object (value));
	else
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
}

PppoaDbGui *
pppoa_db_gui_new (PppoaDb *db)
{
	return PPPOA_DB_GUI (g_object_new (PPPOA_TYPE_DB_GUI, "db", db, NULL));
}

/**** actual code ****/

static void
pppoa_db_gui_populate_countries (PppoaDbGui *self, GtkTreeStore *store)
{
	PppoaDb *db = PPPOA_DB_GUI_GET_PRIVATE (self)->db;
	GSList *countries = pppoa_db_get_countries (db);
	GSList *iter, *isps, *isp_iter;
	PppoaDbEntry *entry;
	gint index = 0;
	

	for (iter = countries; iter; iter = iter->next, index++) {
		GtkTreeIter country_iter;
		const gchar *code;
		
		/* append a new item into the countries store */
		code = (const gchar *) iter->data;
		gtk_tree_store_append (store, &country_iter, NULL);
		gtk_tree_store_set (
			store, &country_iter,
			COL_LABEL, t_code_to_country (code),
			COL_COUNTRY_INDEX, -1,
			COL_COUNTRY_CODE, code,
			-1
		);

		/* setup the isp's store */
		isps = pppoa_db_get_entries (db, code);
		for (isp_iter = isps; isp_iter; isp_iter = isp_iter->next) {
			GtkTreeIter iter;

			entry = (PppoaDbEntry *) isp_iter->data;
			gtk_tree_store_append (store, &iter, NULL);
			gtk_tree_store_set (
				store, &iter,
				COL_LABEL, entry->isp,
				COL_COUNTRY_INDEX, index,
				COL_ISP_ENTRY, entry,
				-1
			);
		}
		g_slist_free (isps);
	}

	g_slist_free (countries);
}

static void
pppoa_db_gui_set_db (PppoaDbGui *self, PppoaDb *db)
{
	GtkTreeStore *store;
	GtkTreeModel *filter_store;
	GtkCellRenderer *cell;
	
	g_return_if_fail (db);
	
	g_object_ref (db);
	
	PPPOA_DB_GUI_GET_PRIVATE (self)->db = db;

	/* setup countries combo box */
	store = gtk_tree_store_new (4, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_POINTER);
	/* populate the store */
	pppoa_db_gui_populate_countries (self, store);
	
	filter_store = gtk_tree_model_filter_new (GTK_TREE_MODEL (store), NULL);
	gtk_tree_model_filter_set_visible_func (
		GTK_TREE_MODEL_FILTER(filter_store),
		(GtkTreeModelFilterVisibleFunc) pppoa_db_gui_is_country_visible,
		self,
		NULL
	);
	self->countries = GTK_COMBO_BOX (gtk_combo_box_new_with_model (filter_store));
	g_object_unref (filter_store);
	g_signal_connect (self->countries, "changed", G_CALLBACK (pppoa_db_gui_on_country_changed), self);
	
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self->countries), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (self->countries), cell,
			"text", COL_LABEL,
			NULL);
	
	filter_store = gtk_tree_model_filter_new (GTK_TREE_MODEL (store), NULL);
	
	gtk_tree_model_filter_set_visible_func (
		GTK_TREE_MODEL_FILTER(filter_store),
		(GtkTreeModelFilterVisibleFunc) pppoa_db_gui_is_isp_visible,
		self,
		NULL
	);
	self->isps = GTK_COMBO_BOX (gtk_combo_box_new_with_model (filter_store));
	g_object_unref (filter_store);

	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self->isps), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (self->isps), cell,
			"text", COL_LABEL,
			NULL);
			
	g_signal_connect (self->isps, "changed", G_CALLBACK (pppoa_db_gui_on_isp_changed), self);
	g_object_unref (store);

}

static gboolean
pppoa_db_gui_is_isp_visible (GtkTreeModel *model, GtkTreeIter *iter, PppoaDbGui *self)
{
	gint country_index;
	gint index;
	index = gtk_combo_box_get_active (self->countries);
	if (index < 0)
		return FALSE;
	
	gtk_tree_model_get (model, iter, COL_COUNTRY_INDEX, &country_index, -1);
	return index == country_index;
}

static gboolean
pppoa_db_gui_is_country_visible (GtkTreeModel *model, GtkTreeIter *iter, PppoaDbGui *self)
{
	gint country_index;
	
	gtk_tree_model_get (model, iter, COL_COUNTRY_INDEX, &country_index, -1);
	return country_index < 0;
}

static void
pppoa_db_gui_on_country_changed (GtkComboBox *cbox, PppoaDbGui *db_gui)
{
	GtkTreeIter iter;
	GtkTreeModelFilter *filter;
	const gchar *code;
	GtkTreeModel *model;
	
	if (gtk_combo_box_get_active_iter (cbox, &iter)) {
		model = gtk_combo_box_get_model (cbox);
		gtk_tree_model_get (model, &iter, COL_COUNTRY_CODE, &code, -1);
		PPPOA_DB_GUI_GET_PRIVATE (db_gui)->code = code;
		filter = GTK_TREE_MODEL_FILTER (gtk_combo_box_get_model (db_gui->isps));
		gtk_tree_model_filter_refilter (filter);
		gtk_combo_box_set_active (db_gui->isps, 0);
	}

}

static void
pppoa_db_gui_on_isp_changed (GtkComboBox *cbox, PppoaDbGui *db_gui)
{
	GtkTreeIter iter;
	PppoaDbEntry *entry;
	GtkTreeModel *model;
	PppoaDbGuiPrivate *p;
	
	if (gtk_combo_box_get_active_iter (cbox, &iter)) {
		model = gtk_combo_box_get_model (cbox);
		/* now we have the iter, we can get it's code */
		gtk_tree_model_get (model, &iter, COL_ISP_ENTRY, &entry, -1);
		p = PPPOA_DB_GUI_GET_PRIVATE (db_gui);
		p->entry = entry;
		g_signal_emit (db_gui, signals[SIG_CHANGED], 0);
	}
}

gint
pppoa_db_gui_get_selected_vpi (PppoaDbGui *self)
{
	PppoaDbEntry *entry = PPPOA_DB_GUI_GET_PRIVATE (self)->entry;
	g_return_val_if_fail (entry, 0);
	return entry->vpi;
}

gint
pppoa_db_gui_get_selected_vci (PppoaDbGui *self)
{
	PppoaDbEntry *entry = PPPOA_DB_GUI_GET_PRIVATE (self)->entry;
	g_return_val_if_fail (entry, 0);
	return entry->vci;
}

const gchar *
pppoa_db_gui_get_selected_code (PppoaDbGui *self)
{
	return PPPOA_DB_GUI_GET_PRIVATE (self)->code;
}

const gchar *
pppoa_db_gui_get_selected_isp (PppoaDbGui *self)
{
	PppoaDbEntry *entry;
	g_return_val_if_fail (PPPOA_IS_DB_GUI (self), NULL);
	entry = PPPOA_DB_GUI_GET_PRIVATE (self)->entry;
	g_return_val_if_fail (entry, 0);
	return entry->isp;
}

gboolean
pppoa_db_gui_activate (PppoaDbGui *self, const gchar *code, const gchar *isp_name)
{
	g_return_val_if_fail (PPPOA_IS_DB_GUI (self), FALSE);
	g_return_val_if_fail (code, FALSE);
	g_return_val_if_fail (isp_name, FALSE);
	
	/* check if we have this country code */
	if (!pppoa_db_get (PPPOA_DB_GUI_GET_PRIVATE (self)->db, code, isp_name))
		return FALSE;
	
	/* now that we have the appropriate isp model we have to find each item */
	g_return_val_if_fail (t_combo_box_set_active_string (self->countries, COL_COUNTRY_CODE, code), FALSE);
	g_return_val_if_fail (t_combo_box_set_active_string (self->isps, COL_LABEL, isp_name), FALSE);
	return TRUE;
}
