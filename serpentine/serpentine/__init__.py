# Copyright (C) 2004 Tiago Cogumbreiro <cogumbreiro@users.sf.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# Authors: Tiago Cogumbreiro <cogumbreiro@users.sf.net>

"""
This is the window widget which will contain the audio mastering widget
defined in audio_widgets.AudioMastering. 
"""

import os, os.path, gtk, gtk.glade, gobject, sys, statvfs

from mastering import AudioMastering
from recording import RecordMusicList, RecordingMedia
import operations, nautilus_burn, gtk_util
from preferences import RecordingPreferences
from operations import MapProxy
import gnome.ui
# TODO make this actually beautiful

class Serpentine (gtk.Window):
	def __init__ (self, data_dir):
		gtk.Window.__init__ (self, gtk.WINDOW_TOPLEVEL)
		self.preferences = RecordingPreferences (data_dir)
		self.masterer = AudioMastering (self.preferences)
		self.masterer.listeners.append (self)
		g = gtk.glade.XML (os.path.join (self.preferences.data_dir, "serpentine.glade"),
		                   "main_window_container")
		self.add (g.get_widget ("main_window_container"))
		self.set_title ("Serpentine")
		self.set_default_size (450, 350)
		
		# Add a file button
		g.get_widget ("add").connect ("clicked", self.add_file)
		g.get_widget ("add_mni").connect ("activate", self.add_file)
		
		# record button
		g.get_widget("burn").connect ("clicked", self.burn)
		
		# masterer widget
		box = self.get_child()
		self.masterer.show()
		box.add (self.masterer)
		
		# preferences
		g.get_widget ("preferences_mni").connect ('activate', self.__on_preferences)
		
		# setup remove buttons
		self.remove = MapProxy ({'menu': g.get_widget ("remove_mni"),
		                         'button': g.get_widget ("remove")})

		self.remove['menu'].connect ("activate", self.remove_file)
		self.remove['button'].connect ("clicked", self.remove_file)
		self.remove.set_sensitive (False)
		
		# setup record button
		self.burn = g.get_widget ("burn")
		self.burn.set_sensitive (False)
		
		# setup clear buttons
		self.clear = MapProxy ({'menu': g.get_widget ("clear_mni"),
		                        'button': g.get_widget ("clear")})
		self.clear['button'].connect ("clicked", self.clear_files)
		self.clear['menu'].connect ("activate", self.clear_files)
		self.clear.set_sensitive (False)
		
		# setup quit menu item
		g.get_widget ("quit_mni").connect ('activate', self.quit)
		self.connect("destroy", self.quit)
		
		# About dialog
		g.get_widget ("about_mni").connect ('activate', self.__on_about)
		
		self.__last_path = None
		self.__add_file = gtk.FileChooserDialog (buttons = (gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_OPEN,gtk.RESPONSE_OK))
		self.__add_file.set_title ('')
		self.__add_file.set_select_multiple (True)
		
		if not self.preferences.drive:
			gtk_util.dialog_warn ("No recording drive found", "No recording drive found on your system, therefore some of Serpentine's functionalities will be disabled.", self)
			g.get_widget ("preferences_mni").set_sensitive (False)
			self.burn.set_sensitive (False)
	
	def __hig_bytes (self, bytes):
		hig_desc = [("GByte", "GBytes"),
		            ("MByte", "MBytes"),
		            ("KByte", "KByte" ),
		            ("byte" , "bytes" )]
		value, strings = self.__decompose_bytes (bytes, 30, hig_desc)
		return "%.1f %s" % (value, self.__plural (value, strings))
	
	def __decompose_bytes (self, bytes, offset, hig_desc):
		if bytes == 0:
			return (0.0, hig_desc[-1:])
		if offset == 0:
			return (float (bytes), hig_desc[-1:])
			
		part = bytes >> offset
		if part > 0:
			sub_part = part ^ ((part >> offset) << offset)
			return ((part * 1024 + sub_part) / 1024.0, hig_desc[0])
		else:
			del hig_desc[0]
			return self.__decompose_bytes (bytes, offset - 10, hig_desc)

	def __plural (self, value, strings):
		if value == 1:
			return strings[0]
		else:
			return strings[1]
		
	def burn (self, *args):
		# Check if we have space available in our cache dir
		secs = 0
		for music in self.masterer.source:
			if not self.preferences.pool.is_available (music['uri']):
				secs += music['duration']
		# 44100hz * 16bit * 2channels / 8bits = 176400 bytes per sec
		size_needed = secs * 176400L
		s = os.statvfs (self.preferences.temporary_dir)
		size_avail = s[statvfs.F_BAVAIL] * long(s[statvfs.F_BSIZE])
		if (size_avail - size_needed) < 0:
			
			gtk_util.dialog_error ("Not enough space on cache directory",
				"Remove some music tracks or make sure your cache location location has enough free space (about %s)." % (self.__hig_bytes(size_needed - size_avail)), self)
			return
	
		if not self.preferences.temporary_dir_is_ok():
			gtk_util.dialog_warn ("Cache directory location unavailable", "Please check if the cache location exists and has writable permissions.", self)
			self.__on_preferences ()
			return
		if gtk_util.dialog_ok_cancel ("Do you want to continue?", "You are about to record a media disk. Canceling a writing operation will make your disk unusable.", self) != gtk.RESPONSE_OK:
			return
		r = RecordingMedia (self.masterer.source, self.preferences, self)
		r.start()
		
	def quit (self, *args):
		self.preferences.pool.clear()
		gtk.main_quit()
		sys.exit (0)
		
	def on_selection_changed (self, *args):
		self.remove.set_sensitive (self.masterer.count_selected() > 0)
		
	def on_contents_changed (self, *args):
		is_sensitive = len(self.masterer.source) > 0
		self.clear.set_sensitive (is_sensitive)
		self.burn.set_sensitive (is_sensitive)

	def remove_file (self, *args):
		self.masterer.remove_selected()
		
	def clear_files (self, *args):
		self.masterer.source.clear()
		
	def add_file (self, *args):
		if self.__add_file.run () == gtk.RESPONSE_OK:
			files = self.__add_file.get_uris()
			self.masterer.add_files (files)
		self.__add_file.unselect_all()
		self.__add_file.hide()
	
	def __on_preferences (self, *args):
		self.preferences.dialog.run ()
		self.preferences.dialog.hide ()
	
	def __on_about (self, *args):
		a = gnome.ui.About ("Serpentine", self.preferences.version, "Copyright 2004 Tiago Cogumbreiro", "Audio CD Recording", ["Tiago Cogumbreiro <cogumbreiro@users.sf.net>"], [], "")
		a.run ()

if __name__ == '__main__':
	s = Serpentine ()
	s.preferences.simulate = len(sys.argv) == 2 and sys.argv[1] == '--simulate'
	s.show()
	gtk.main()
