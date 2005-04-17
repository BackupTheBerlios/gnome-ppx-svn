/*
 * Copyright (C) 2003 Sun Microsystems, Inc.
 * Copyright (C) 2004 Red Hat Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Authors:
 *    Erwann Chenede  <erwann.chenede@sun.com>
 *    Mark McLoughlin  <mark@skynet.ie>  
 *    Joe Marcus Clarke  <marcus@freebsd.org>
 */

#include "netstatus-sysdeps.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>

static inline gboolean
parse_stats (char  *buf,
	     int    prx_idx,
	     int    ptx_idx,
	     long  *in_packets,
	     long  *out_packets,
	     int    brx_idx,
	     int    btx_idx,
	     long  *in_bytes,
	     long  *out_bytes)
{
  char *p;
  int   i;

  p = strtok (buf, " \t\n");
  for (i = 0; p; i++, p = strtok (NULL, " \t\n"))
    {
      if (i == prx_idx)
	*in_packets = g_ascii_strtoull (p, NULL, 10);
      if (i == ptx_idx)
	*out_packets = g_ascii_strtoull (p, NULL, 10);
      if (i == brx_idx)
	*in_bytes = g_ascii_strtoull (p, NULL, 10);
      if (i == btx_idx)
	*out_bytes = g_ascii_strtoull (p, NULL, 10);
    }

  if (i <= prx_idx || i <= ptx_idx || i <= brx_idx || i <=btx_idx)
    return FALSE;

  return TRUE;
}

#if !defined (__FreeBSD__)

static inline char *
parse_iface_name (const char *buf)
{
  char *p1;

  if ((p1 = strchr (buf, ':')))
    {
      char *p2;

      p2 = strchr (p1, ':');
      if (p2)
	*p2++ = '\0';
      else
	*p1++ = '\0';

      return p2 ? p2 : p1;
    }
  else if ((p1 = strchr (buf, ' ')))
    {
      *p1++ = '\0';
      return p1;
    }

  return NULL;
}

static inline void
parse_stats_header (char *buf,
		    int  *prx_idx,
		    int  *ptx_idx,
		    int  *brx_idx,
		    int  *btx_idx)
{
  char *p;
  int   i;

  *prx_idx = *ptx_idx = -1;
  *brx_idx = *btx_idx = -1;

  p = strtok (buf, "| \t\n");
  p = strtok (NULL, "| \t\n"); /* Skip the first one */
  for (i = 0; p; i++, p = strtok (NULL, "| \t\n"))
    {
      if (!strcmp (p, "packets"))
	{
	  if (*prx_idx == -1)
	    *prx_idx = i;
	  else
	    *ptx_idx = i;
	}
      else if (!strcmp (p, "bytes"))
	{
	  if (*brx_idx == -1)
	    *brx_idx = i;
	  else
	    *btx_idx = i;
	}
    }
}

static inline FILE *
get_proc_net_dev_fh (void)
{
  static FILE *retval = NULL;

  if (retval != NULL)
    return retval;

  return retval = fopen ("/proc/net/dev", "r");
}

gboolean
netstatus_sysdeps_read_iface_statistics (const char  *iface,
					 long        *in_packets,
					 long        *out_packets,
					 long        *in_bytes,
					 long        *out_bytes)
{
  FILE *fh;
  char  buf [512];
  int   prx_idx, ptx_idx;
  int   brx_idx, btx_idx;
  gboolean ret = TRUE;

  g_return_val_if_fail (iface != NULL, FALSE);
  g_return_val_if_fail (in_packets != NULL, FALSE);
  g_return_val_if_fail (out_packets != NULL, FALSE);
  g_return_val_if_fail (in_bytes != NULL, FALSE);
  g_return_val_if_fail (out_bytes != NULL, FALSE);
  
  *in_packets  = -1;
  *out_packets = -1;
  *in_bytes    = -1;
  *out_bytes   = -1;

  fh = get_proc_net_dev_fh ();
  if (!fh)
    return FALSE;

  fgets (buf, sizeof (buf), fh);
  fgets (buf, sizeof (buf), fh);

  parse_stats_header (buf, &prx_idx, &ptx_idx, &brx_idx, &btx_idx);
  if (prx_idx == -1 || ptx_idx == -1 ||
      brx_idx == -1 || btx_idx == -1)
    return FALSE;

  while (fgets (buf, sizeof (buf), fh))
    {
      char *stats;
      char *name;

      name = buf;
      while (g_ascii_isspace (name [0]))
	name++;

      stats = parse_iface_name (name);
      if (!stats)
	{
	  ret = FALSE;
	  continue;
	}

      if (strcmp (name, iface) != 0)
	continue;

      if (!parse_stats (stats,
			prx_idx, ptx_idx, in_packets, out_packets,
			brx_idx, btx_idx, in_bytes, out_bytes))
	{
	  ret = FALSE;
	  continue;
	}

      break;
    }

  if ((*in_packets == -1 || *out_packets == -1 || *in_bytes == -1 || *out_bytes == -1))
    ret = FALSE;

  rewind (fh);
  fflush (fh);

  return ret;
}

#else /* defined(__FreeBSD__) */

static inline void
parse_header (char *buf,
	      int  *prx_idx,
	      int  *ptx_idx,
	      int  *brx_idx,
	      int  *btx_idx)
{
   char *p;
   int   i;

   *prx_idx = *ptx_idx = -1;
   *brx_idx = *btx_idx = -1;

   p = strtok (buf, " \n\t");
   for (i = 0; p; i++, p = strtok (NULL, " \t\n"))
     {
        if (!strcmp (p, "Ipkts"))
	  {
	     *prx_idx = i;
	  }
	else if (!strcmp (p, "Ibytes"))
	  {
	     *brx_idx = i;
	  }
	else if (!strcmp (p, "Opkts"))
	  {
	     *ptx_idx = i;
	  }
	else if (!strcmp (p, "Obytes"))
	  {
	     *btx_idx = i;
	  }
     }
}

gboolean
netstatus_sysdeps_read_iface_statistics (const char *iface,
					 long       *in_packets,
					 long       *out_packets,
					 long       *in_bytes,
					 long       *out_bytes)
{
  GError  *error;
  char    *command_line;
  char   **argv;
  gboolean ret = TRUE;
  int      pipe_out;

  g_return_val_if_fail (iface != NULL, NULL);
  g_return_val_if_fail (in_packets != NULL, NULL);
  g_return_val_if_fail (out_packets != NULL, NULL);
  g_return_val_if_fail (in_bytes != NULL, NULL);
  g_return_val_if_fail (out_bytes != NULL, NULL);

  *in_packets  = -1;
  *out_packets = -1;
  *in_bytes    = -1;
  *out_bytes   = -1;

  error = NULL;
  command_line = g_strdup_printf ("/usr/bin/netstat -n -I %s -b -f inet", iface);
  if (!g_shell_parse_argv (command_line, NULL, &argv, &error))
    {
      g_error_free (error);
      g_free (command_line);
      
      return FALSE;
    }
  g_free (command_line);

  error = NULL;
  if (g_spawn_async_with_pipes (NULL,
				argv,
				NULL,
				0,
				NULL,
				NULL,
				NULL,
				NULL,
				&pipe_out,
				NULL,
				&error))
    {
      GIOChannel *channel;
      char       *buf;
      int         prx_idx, ptx_idx;
      int         brx_idx, btx_idx;

      channel = g_io_channel_unix_new (pipe_out);

      g_io_channel_read_line (channel, &buf, NULL, NULL, NULL);
      parse_header (buf, &prx_idx, &ptx_idx, &brx_idx, &btx_idx);
      g_free (buf);

      if (prx_idx == -1 || ptx_idx == -1 ||
	  brx_idx == -1 || btx_idx == -1)
	{
	  ret = FALSE;
	  goto error_shutdown;
	}

      g_io_channel_read_line (channel, &buf, NULL, NULL, NULL);

      if (!parse_stats (buf,
			prx_idx, ptx_idx, in_packets, out_packets,
			brx_idx, btx_idx, in_bytes, out_bytes))
	{
	  ret = FALSE;
	}
      else if (*in_packets == -1 || *out_packets == -1 || *in_bytes == -1 || *out_bytes == -1)
	{
	  ret = FALSE;
	}

      g_free (buf);

    error_shutdown:
      g_io_channel_unref (channel);
      close (pipe_out);
    }
  else
    {
      ret = FALSE;
    }

  g_strfreev (argv);

  return ret;
}

#endif /* !defined(__FreeBSD__) */
