#ifndef __PPP_PROGRESS_H__
#define __PPP_PROGRESS_H__

#include <glib.h>
#include <glib-object.h>
#include "ppp_manager.h"

G_BEGIN_DECLS

#define PPP_TYPE_PROGRESS            (ppp_progress_get_type())
#define PPP_PROGRESS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PPP_TYPE_PROGRESS, PppProgress))
#define PPP_PROGRESS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PPP_TYPE_PROGRESS, PppProgressClass))
#define PPP_IS_PROGRESS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PPP_TYPE_PROGRESS))
#define PPP_IS_PROGRESS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),PPP_TYPE_PROGRESS))
#define PPP_PROGRESS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PPP_TYPE_PROGRESS, PppProgressClass))

typedef struct _PppProgress PppProgress;
typedef struct _PppProgressClass PppProgressClass;

struct _PppProgressClass {
	GObjectClass parent_class;
};

struct _PppProgress {
	GObject parent;
};

GType ppp_progress_get_type (void);
PppProgress* ppp_progress_new (PppManager *manager);

G_END_DECLS

#endif /* __PPP_PROGRESS_H__ */
