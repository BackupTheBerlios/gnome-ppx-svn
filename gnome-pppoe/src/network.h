#ifndef __NETWORK_H_
#define __NETWORK_H_
#include <glib.h>
GList* list_ifaces (void);
GList* list_ethernet_ifaces(void);
int iface_get_type (char *iface);
#endif
