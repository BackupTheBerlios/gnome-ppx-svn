#ifndef __PPPOA_DB_GUI_H__
#define __PPPOA_DB_GUI_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "pppoa_db.h"

G_BEGIN_DECLS

#define PPPOA_TYPE_DB_GUI            (pppoa_db_gui_get_type ())
#define PPPOA_DB_GUI(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PPPOA_TYPE_DB_GUI, PppoaDbGui))
#define PPPOA_DB_GUI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PPPOA_TYPE_DB_GUI, PppoaDbGuiClass))
#define PPPOA_IS_DB_GUI(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPPOA_TYPE_DB_GUI))
#define PPPOA_IS_DB_GUI_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PPPOA_TYPE_DB_GUI))
#define PPPOA_DB_GUI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PPPOA_TYPE_DB_GUI, PppoaDbGuiClass))

typedef struct _PppoaDbGui PppoaDbGui;
typedef struct _PppoaDbGuiClass PppoaDbGuiClass;

struct _PppoaDbGuiClass {
	GObjectClass parent;
	/* signal is emited when the vpi and vci values have changed */
	void (*values_changed) (PppoaDbGui *self);
};

struct _PppoaDbGui {
	GObject parent;
	GtkComboBox *countries;
	GtkComboBox *isps;
};

GType pppoa_db_gui_get_type (void);

PppoaDbGui *pppoa_db_gui_new (PppoaDb *db);
gint pppoa_db_gui_get_selected_vpi (PppoaDbGui *self);
gint pppoa_db_gui_get_selected_vci (PppoaDbGui *self);
const gchar *pppoa_db_gui_get_selected_code (PppoaDbGui *self);
const gchar *pppoa_db_gui_get_selected_isp (PppoaDbGui *self);
/* activates a given isp in a given country 
 * values_changed will be emited. Returns NULL if the isp was not found.
 */
gboolean pppoa_db_gui_activate (PppoaDbGui *self, const gchar *code, const gchar *isp_name);

G_END_DECLS
#endif /* __PPPOA_DB_GUI_H__ */
