/* 
 * Copyright (C) 2004 Tiago Cogumbreiro <cogumbreiro@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors: Tiago Cogumbreiro <cogumbreiro@users.sf.net>
 */

/* Generated data (by glib-mkenums) */

#include <cd-drive.h>
#include <bacon-cd-selection.h>
#include <cd-recorder.h>

/* enumerations from "/usr/include/libnautilus-burn/cd-drive.h" */
GType
cd_media_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { CD_MEDIA_TYPE_BUSY, "CD_MEDIA_TYPE_BUSY", "busy" },
      { CD_MEDIA_TYPE_ERROR, "CD_MEDIA_TYPE_ERROR", "error" },
      { CD_MEDIA_TYPE_UNKNOWN, "CD_MEDIA_TYPE_UNKNOWN", "unknown" },
      { CD_MEDIA_TYPE_CD, "CD_MEDIA_TYPE_CD", "cd" },
      { CD_MEDIA_TYPE_CDR, "CD_MEDIA_TYPE_CDR", "cdr" },
      { CD_MEDIA_TYPE_CDRW, "CD_MEDIA_TYPE_CDRW", "cdrw" },
      { CD_MEDIA_TYPE_DVD, "CD_MEDIA_TYPE_DVD", "dvd" },
      { CD_MEDIA_TYPE_DVDR, "CD_MEDIA_TYPE_DVDR", "dvdr" },
      { CD_MEDIA_TYPE_DVDRW, "CD_MEDIA_TYPE_DVDRW", "dvdrw" },
      { CD_MEDIA_TYPE_DVD_RAM, "CD_MEDIA_TYPE_DVD_RAM", "dvd-ram" },
      { CD_MEDIA_TYPE_DVD_PLUS_R, "CD_MEDIA_TYPE_DVD_PLUS_R", "dvd-plus-r" },
      { CD_MEDIA_TYPE_DVD_PLUS_RW, "CD_MEDIA_TYPE_DVD_PLUS_RW", "dvd-plus-rw" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("CDMediaType", values);
  }
  return etype;
}

GType
cd_drive_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { CDDRIVE_TYPE_FILE, "CDDRIVE_TYPE_FILE", "file" },
      { CDDRIVE_TYPE_CD_RECORDER, "CDDRIVE_TYPE_CD_RECORDER", "cd-recorder" },
      { CDDRIVE_TYPE_CDRW_RECORDER, "CDDRIVE_TYPE_CDRW_RECORDER", "cdrw-recorder" },
      { CDDRIVE_TYPE_DVD_RAM_RECORDER, "CDDRIVE_TYPE_DVD_RAM_RECORDER", "dvd-ram-recorder" },
      { CDDRIVE_TYPE_DVD_RW_RECORDER, "CDDRIVE_TYPE_DVD_RW_RECORDER", "dvd-rw-recorder" },
      { CDDRIVE_TYPE_DVD_PLUS_R_RECORDER, "CDDRIVE_TYPE_DVD_PLUS_R_RECORDER", "dvd-plus-r-recorder" },
      { CDDRIVE_TYPE_DVD_PLUS_RW_RECORDER, "CDDRIVE_TYPE_DVD_PLUS_RW_RECORDER", "dvd-plus-rw-recorder" },
      { CDDRIVE_TYPE_CD_DRIVE, "CDDRIVE_TYPE_CD_DRIVE", "cd-drive" },
      { CDDRIVE_TYPE_DVD_DRIVE, "CDDRIVE_TYPE_DVD_DRIVE", "dvd-drive" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("CDDriveType", values);
  }
  return etype;
}


/* enumerations from "/usr/include/libnautilus-burn/cd-recorder.h" */
GType
track_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { TRACK_TYPE_AUDIO, "TRACK_TYPE_AUDIO", "audio" },
      { TRACK_TYPE_DATA, "TRACK_TYPE_DATA", "data" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("TrackType", values);
  }
  return etype;
}

GType
cd_recorder_write_flags_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { CDRECORDER_EJECT, "CDRECORDER_EJECT", "eject" },
      { CDRECORDER_BLANK, "CDRECORDER_BLANK", "blank" },
      { CDRECORDER_DUMMY_WRITE, "CDRECORDER_DUMMY_WRITE", "dummy-write" },
      { CDRECORDER_DISC_AT_ONCE, "CDRECORDER_DISC_AT_ONCE", "disc-at-once" },
      { CDRECORDER_DEBUG, "CDRECORDER_DEBUG", "debug" },
      { CDRECORDER_OVERBURN, "CDRECORDER_OVERBURN", "overburn" },
      { CDRECORDER_BURNPROOF, "CDRECORDER_BURNPROOF", "burnproof" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("CDRecorderWriteFlags", values);
  }
  return etype;
}

GType
cd_recorder_actions_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PREPARING_WRITE, "PREPARING_WRITE", "preparing-write" },
      { WRITING, "WRITING", "writing" },
      { FIXATING, "FIXATING", "fixating" },
      { BLANKING, "BLANKING", "blanking" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("CDRecorderActions", values);
  }
  return etype;
}

GType
cd_recorder_media_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { MEDIA_CD, "MEDIA_CD", "cd" },
      { MEDIA_DVD, "MEDIA_DVD", "dvd" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("CDRecorderMedia", values);
  }
  return etype;
}


/* Generated data ends here */

