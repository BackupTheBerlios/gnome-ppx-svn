#include <sys/ioctl.h>
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif /* HAVE_SYS_SOCKIO_H */
#include <sys/param.h>
#include <net/if.h>
#include <net/if_arp.h>
#define PROC_NET_DEV           "/proc/net/dev"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "network.h"
#if !defined(HAVE_SOCKADDR_SA_LEN)
#define NETSTATUS_SA_LEN(saddr) (sizeof (struct sockaddr))
#else
#define NETSTATUS_SA_LEN(saddr) (MAX ((saddr)->sa_len, sizeof (struct sockaddr)))
#endif /* HAVE_SOCKADDR_SA_LEN */

#include <libgnome/gnome-i18n.h>

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

static GList* list_proc_ifaces(void)
{
    FILE *fh;
    char buf[512];
	GList *ifaces = NULL;
	char *s, name[IFNAMSIZ];
	
	fh = fopen(PROC_NET_DEV, "r");
	if (!fh) {
		return NULL;
	}
	
	/* skip header */
	fgets(buf, sizeof buf, fh);
	fgets(buf, sizeof buf, fh);
	
	while (fgets(buf, sizeof buf, fh)) {
		s = get_name(name, buf);
		ifaces = g_list_append(ifaces, g_strdup(name));
	}
	fclose(fh);
	return ifaces;
}

static GList * list_ifconf_ifaces(void)
{
	char buff[1024];
	int skfd;
	struct ifconf ifc;
	struct ifreq *ifr;
	int i;
	GList *ifaces = NULL;

	ifc.ifc_len = sizeof(buff);
	ifc.ifc_buf = buff;
	skfd = socket (AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0 || ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
		return NULL;
	}
	
	ifr = ifc.ifc_req;
	for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; ifr++) {
		ifaces = g_list_append(ifaces, g_strdup(ifr->ifr_name));
	}
	return ifaces;
}

GList* list_ifaces(void)
{
	GList *ret = list_proc_ifaces();
	return ret ? ret : list_ifconf_ifaces();
}

int iface_get_type (char *iface)
{
#ifdef SIOCGIFHWADDR
  struct ifreq          if_req;
  int                   fd;
  
  if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      g_warning (_(": unable to open AF_INET socket: %s\n"),
		 g_strerror (errno));
      return 0;
    }
  
  strncpy (if_req.ifr_name, iface, IF_NAMESIZE - 1);
  if_req.ifr_name [IF_NAMESIZE - 1] = '\0';
  if (ioctl (fd, SIOCGIFHWADDR, &if_req) < 0)
    {
      g_warning (_(": unable to obtain hardware address: %s\n"),
		 g_strerror (errno));
      close (fd);
      return 0;
    }

  close (fd);
	return if_req.ifr_hwaddr.sa_family;
#else
	return 0;
#endif
}

GList* list_ethernet_ifaces(void)
{
	GList *iter, *ret;
	ret = NULL;
	for(iter = g_list_first(list_ifaces()); iter; iter = iter->next) {
		if( iface_get_type(iter->data) == ARPHRD_ETHER) {
			ret = g_list_append(ret, iter->data);
			iter->data = NULL;
		} else {
			g_free(iter->data);
			iter->data = NULL;
		}
	}
	g_list_free(iter);
	return ret;
}
