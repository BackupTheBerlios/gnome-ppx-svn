
#include "ppp_status.h"

#include <gtk/gtk.h>
#include <time.h>
#include <glib/gi18n.h>
#include "ppp_marshallers.h"
#include "netstatus-sysdeps.h"
#include "ppp_state_listener.h"
#include <pppd.h>
#include <tlib/tlib.h>

#define PPP_STATUS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PPP_TYPE_STATUS, PppStatusPrivate))

typedef struct _PppStatusPrivate PppStatusPrivate;

/* gobject prototypes */
static void ppp_status_set_property(GObject * object, guint prop_id,
				      const GValue * value,
				      GParamSpec * pspec);

/* state listener prototypes */
static void ppp_status_on_idle(PppStateListener * self, GObject * origin);
static void ppp_status_on_connected(PppStateListener * self,
				    GObject * origin,
				    gboolean new_connection);
static void ppp_status_on_tray_usage(PppStateListener * self,
				     GObject * origin, gboolean use_tray);
static void ppp_status_state_listener_init(PppStateListenerIface *
					   iface);

/* prototypes */
static void ppp_status_close (GtkWidget *w, PppStatusPrivate *p);
static gboolean ppp_status_delete (GtkWidget *w, GdkEvent *e, PppStatusPrivate *p);
static void ppp_status_disconnect (GtkWidget *button, PppStatusPrivate *p);
static void ppp_status_set_glade_file(PppStatus * self, const gchar * glade_file);

struct _PppStatusPrivate {
	PppStatus *self;
	gboolean looping;
	PppManager *manager;
	/* details window */
	GtkLabel  *interface;
	GtkLabel  *address;
	GtkLabel  *netmask;
	GtkLabel  *duration;
	GtkLabel  *sent;
	GtkLabel  *recieved;
	GtkWidget *close;
};

enum {
	PROP_0,
	PROP_MANAGER,
};

G_DEFINE_TYPE_WITH_CODE (PppStatus, ppp_status, GTK_TYPE_WINDOW,
	G_IMPLEMENT_INTERFACE (
		PPP_TYPE_STATE_LISTENER,
		ppp_status_state_listener_init
	)
);

static void ppp_status_class_init(PppStatusClass * klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS(klass);
	
	g_type_class_add_private(gobject_class, sizeof (PppStatusPrivate));
	
	gobject_class->set_property = ppp_status_set_property;
	
	/* class constructor arguments */
	g_object_class_install_property (gobject_class, PROP_MANAGER,
					g_param_spec_object("manager",
							    "manager",
							    "manager",
							    PPP_TYPE_MANAGER,
							    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

GtkWidget *ppp_status_new(PppManager *manager)
{
	GObject *self;
	
	self = g_object_new(
		PPP_TYPE_STATUS,
		"manager", manager,
		"title", _("Connection Properties"),
		"type", GTK_WINDOW_TOPLEVEL,
		NULL
	);
	
	return GTK_WIDGET (self);
}

static void ppp_status_init(PppStatus * object)
{
	PppStatusPrivate *private_data;

	private_data = PPP_STATUS_GET_PRIVATE(object);
	private_data->self = object;
	private_data->looping = FALSE;
	g_signal_connect (object, "delete-event", G_CALLBACK(ppp_status_delete), private_data);
}

static void ppp_status_state_listener_init(PppStateListenerIface *
					   iface)
{
	iface->on_idle       = ppp_status_on_idle;
	iface->on_connecting = (PSLOnConnectingFunc) gtk_widget_hide;
	iface->on_startup    = (PSLOnStartupFunc) gtk_true;
	iface->on_connected  = ppp_status_on_connected;
	iface->on_tray_usage = ppp_status_on_tray_usage;
}

static void ppp_status_set_property(GObject * object, guint prop_id,
				    const GValue * value,
				    GParamSpec * pspec)
{
	PppStatus *self;
	PppManager *manager;

	self = PPP_STATUS(object);
	
	switch (prop_id) {
		case PROP_MANAGER:
			manager = g_value_get_object(value);
			PPP_STATUS_GET_PRIVATE (self)->manager = manager;
			ppp_status_set_glade_file(self, ppp_manager_get_glade_file (manager));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

/********** end of boiler plate code ************/

static void ppp_status_set_glade_file(PppStatus * self, const gchar * glade_file)
{
	PppStatusPrivate *p = PPP_STATUS_GET_PRIVATE(self);
	const TGladeInfo nfo[] = {
		{"ppp_status", GTK_TYPE_CONTAINER},
		{"interface",  GTK_TYPE_LABEL},
		{"address",    GTK_TYPE_LABEL},
		{"netmask",    GTK_TYPE_LABEL},
		{"duration",   GTK_TYPE_LABEL},
		{"sent",       GTK_TYPE_LABEL},
		{"recieved",   GTK_TYPE_LABEL},
		{"close",      GTK_TYPE_BUTTON},
		{NULL}
	};
	const TGladeSignal sig[] = {
		{"ppp_status_disconnect", G_CALLBACK(ppp_status_disconnect), p},
		{"ppp_status_close",      G_CALLBACK(ppp_status_close),      p},
		{NULL}
	};
	GHashTable *ht;
	GError *err = NULL;
	
	ht = t_glade_parse (glade_file, NULL, nfo, sig, &err);
	T_ERR_RETURN (err);
	
	gtk_container_add (GTK_CONTAINER (self), g_hash_table_lookup (ht, "ppp_status"));
	
	p->interface = GTK_LABEL  (g_hash_table_lookup (ht, "interface"));
	p->address   = GTK_LABEL  (g_hash_table_lookup (ht, "address"));
	p->netmask   = GTK_LABEL  (g_hash_table_lookup (ht, "netmask"));
	p->duration  = GTK_LABEL  (g_hash_table_lookup (ht, "duration"));
	p->sent      = GTK_LABEL  (g_hash_table_lookup (ht, "sent"));
	p->recieved  = GTK_LABEL  (g_hash_table_lookup (ht, "recieved"));
	p->close     = GTK_WIDGET (g_hash_table_lookup (ht, "close"));
	
	g_hash_table_destroy (ht);
	
	gtk_container_set_border_width (GTK_CONTAINER (self), 12);
	
}

/*************** end of getter/setter's ************/

#define ONE_GIGA (1 << 30)
#define ONE_MEGA (1 << 20)
#define ONE_KILO (1 << 10)

static gchar *ppp_status_humanize(unsigned long bytes)
{
	int high, low;
	double val;
	
	val = bytes / (double) ONE_GIGA;
	if (val >= 1) {
		return g_strdup_printf ("%.2f Gb", val);
	}
	
	val = bytes / (double) ONE_MEGA;
	if (val >= 1)
		return g_strdup_printf ("%.2f Mb", val);
	
	val = bytes / (double) ONE_KILO;
	if (val >= 1)
		return g_strdup_printf ("%.2f Kb", val);
	
	return g_strdup_printf ("%u b", (unsigned int) bytes);

	high = bytes / ONE_GIGA;
	if (high) {
		low = bytes % ONE_GIGA;
		return g_strdup_printf ("%d.%d Gb", high, low);
	}
	
	high = bytes / ONE_MEGA;
	if (high) {
		low = bytes % ONE_MEGA;
		return g_strdup_printf ("%d.%d Mb", high, low); 
	}
	
	high = bytes / ONE_KILO;
	if (high) {
		low = bytes % ONE_KILO;
		return g_strdup_printf ("%d.%d Kb", high, low);
	}
	
	return g_strdup_printf ("%ld b", bytes);
}

static gboolean ppp_status_update_time (gpointer data)
{
	PppStatusPrivate *p = (PppStatusPrivate *) data;
	time_t curr;
	int elapsed, hours, mins, secs;
	gchar *str;
	long in_packets, out_packets, in_bytes, out_bytes;
	pppd_monitor *monitor = ppp_manager_get_monitor (p->manager);
	
	time(&curr);
	elapsed = (int) difftime(curr, pppd_monitor_get_time(monitor));
	secs = elapsed % 60;
	elapsed /= 60;
	mins = elapsed % 60;
	elapsed /= 60;
	hours = elapsed;
	str = g_strdup_printf("%.2d:%.2d:%.2d", hours, mins, secs);
	gtk_label_set_text(p->duration, str);
	g_free(str);
	netstatus_sysdeps_read_iface_statistics(pppd_monitor_get_interface(monitor), &in_packets, &out_packets, &in_bytes, &out_bytes);
	str = ppp_status_humanize(in_bytes);
	gtk_label_set_text(p->recieved, str);
	g_free(str);
	str = ppp_status_humanize(out_bytes);
	gtk_label_set_text(p->sent, str);
	g_free(str);
	return TRUE;
}


static void ppp_status_close (GtkWidget *w, PppStatusPrivate *p)
{
	if (ppp_manager_use_tray (p->manager))
		gtk_widget_hide (GTK_WIDGET (p->self));
	else
		ppp_manager_quit (p->manager, TRUE);
}

static gboolean ppp_status_delete (GtkWidget *w, GdkEvent *e, PppStatusPrivate *p)
{
	ppp_status_close (NULL, p);
	return TRUE;
}

static void ppp_status_disconnect (GtkWidget *button, PppStatusPrivate *p)
{
	ppp_manager_disconnect (p->manager, TRUE);
}

static void ppp_status_on_connected (PppStateListener* self, GObject* origin, gboolean new_connection)
{
	PppStatusPrivate *p;
	gchar *str;
	guchar ip[4], netmask[4];
	p = PPP_STATUS_GET_PRIVATE (self);
	ppp_status_on_tray_usage (self, origin, ppp_manager_use_tray (PPP_MANAGER (origin)));
	/* we need to update the stats now */
	gtk_label_set_text (p->interface, pppd_monitor_get_interface (ppp_manager_get_monitor (p->manager)));
	pppd_monitor_get_ip (ppp_manager_get_monitor (p->manager), ip, netmask);
	str = g_strdup_printf ("%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	gtk_label_set_text (p->address, str);
	g_free(str);
	str = g_strdup_printf ("%d.%d.%d.%d", netmask[0], netmask[1], netmask[2], netmask[3]);
	gtk_label_set_text (p->netmask, str);
	g_free (str);
	ppp_status_update_time (p);
	p->looping = TRUE;
	g_timeout_add (1000, ppp_status_update_time, p);
}

static void ppp_status_on_idle(PppStateListener * self, GObject * origin)
{
	PppStatusPrivate *p;
	p = PPP_STATUS_GET_PRIVATE (self);
	if (p->looping) {
		g_idle_remove_by_data (p);
		p->looping = FALSE;
	}
	gtk_widget_hide (GTK_WIDGET (self));
}

static void ppp_status_on_tray_usage(PppStateListener * self,
				     GObject * origin, gboolean use_tray)
{
	pppd_monitor *monitor;
	monitor = ppp_manager_get_monitor (PPP_STATUS_GET_PRIVATE (self)->manager);
	if (!use_tray &&  PPPD_MONITOR_CONNECTED == pppd_monitor_get_state (monitor))
		gtk_widget_show (GTK_WIDGET (self));
}
