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

import os, os.path, gtk, gtk.glade, gobject, sys, statvfs, gnome.ui
from xml.parsers.expat import ExpatError

# Private modules
import operations, nautilusburn, gtkutil
from mastering import AudioMastering
from recording import RecordMusicList, RecordingMedia
from preferences import RecordingPreferences
from operations import MapProxy, OperationListener
import constants

try:
	import services, dbus
except ImportError:
	services = None

class SerpentineError (StandardError): pass

class Serpentine (gtk.Window, OperationListener):
	def __init__ (self, data_dir = None):
		if services:
			bus = dbus.SessionBus ()
			dbus_srv = bus.get_service ("org.freedesktop.DBus")
			dbus_obj = dbus_srv.get_object ("/org/freedesktop/DBus", "org.freedesktop.DBus")
			if services.SERVICE_NAME in dbus_obj.ListServices ():
				raise SerpentineError("Serpentine is already running.")
			
			# Start the service since it isn't available
			self.__service = dbus.Service (services.SERVICE_NAME, bus=bus)
			self.__application = services.Application (self.__service, self)
				
		self.__recording = []
		self.__initialized = False
		gtk.Window.__init__ (self, gtk.WINDOW_TOPLEVEL)

		if data_dir is None:
			data_dir = constants.data_dir
		self.preferences = RecordingPreferences (data_dir)
		self.preferences.dialog.set_transient_for (self)
		self.masterer = AudioMastering (self.preferences)
		self.masterer.listeners.append (self)

		glade_file = os.path.join (data_dir, "serpentine.glade")
		g = gtk.glade.XML (glade_file, "main_window_container")
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

		self.remove["menu"].connect ("activate", self.remove_file)
		self.remove["button"].connect ("clicked", self.remove_file)
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
		self.connect("delete-event", self.quit)
		
		# About dialog
		g.get_widget ("about_mni").connect ('activate', self.__on_about)
		
		self.__last_path = None
		self.__add_file = gtk.FileChooserDialog (buttons = (gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_OPEN,gtk.RESPONSE_OK))
		self.__add_file.set_title ('')
		self.__add_file.set_select_multiple (True)
		
		# update buttons
		self.on_contents_changed()
		
		if self.preferences.drive is None:
			gtkutil.dialog_warn ("No recording drive found", "No recording drive found on your system, therefore some of Serpentine's functionalities will be disabled.", self)
			g.get_widget ("preferences_mni").set_sensitive (False)
			self.burn.set_sensitive (False)
		self.__load_playlist()
	
	recording = property (lambda self: self.__recording)
	
	def __on_show (self, *args):
		if self.__initialized:
			return
		self.set_sensitive(True)
		gobject.idle_add (self.__load_playlist)
		self.__initialized = True
	
	def __load_playlist (self):
		try:
			self.preferences.load_playlist (self.masterer.source)
		except ExpatError:
			pass
		except IOError:
			pass
			
			
	def burn (self, *args):
		if not self.preferences.temporary_dir_is_ok():
			gtkutil.dialog_warn ("Cache directory location unavailable",
			                     "Please check if the cache location exists and has writable permissions.",
			                     self)
			self.__on_preferences ()
			return
			
		# Check if we have space available in our cache dir
		secs = 0
		for music in self.masterer.source:
			if not self.preferences.pool.is_available (music["location"]):
				secs += music['duration']
		# 44100hz * 16bit * 2channels / 8bits = 176400 bytes per sec
		size_needed = secs * 176400L
		try:
			s = os.statvfs (self.preferences.temporary_dir)
		except OSError:
			gtkutil.dialog_warn ("Cache directory location unavailable", "Please check if the cache location exists and has writable permissions.", self)
			self.__on_preferences ()
			return
			
		size_avail = s[statvfs.F_BAVAIL] * long(s[statvfs.F_BSIZE])
		if (size_avail - size_needed) < 0:
			
			gtkutil.dialog_error ("Not enough space on cache directory",
				"Remove some music tracks or make sure your cache location location has enough free space (about %s)." % (self.__hig_bytes(size_needed - size_avail)), self)
			return
	
		if self.masterer.source.total_duration > self.masterer.disk_size:
			title = "Do you want to overburn your disk?"
			msg = "You are about to record a media disk in overburn mode." + " " + \
			      "This may not work on all drives and shouldn't give you more then a couple of minutes."
			btn = "Overburn Disk"
			self.preferences.overburn = True
		else:
			title = "Do you want to record your musics?"
			msg = "You are about to record a media disk." + " " + \
			      "Canceling a writing operation will make your disk unusable."
			btn = "Record Disk"
			self.preferences.overburn = False
		
		if gtkutil.dialog_ok_cancel (title, msg, self, btn) != gtk.RESPONSE_OK:
			return
		r = RecordingMedia (self.masterer.source, self.preferences, self)
		
		r.start()
		r.listeners.append (self)
		self.burn.set_sensitive (False)
		dev = r.drive.get_device ()
		assert dev not in self.__recording
		self.__recording.append (dev)
		
	def quit (self, *args):
		if self.recording:
			gtkutil.dialog_warn ("Stop recording first", "Serpentine can only exit if you cancel the recording operation first.", self)
			return True
		self.preferences.save_playlist (self.masterer.source)
		self.preferences.pool.clear()
		gtk.main_quit()
		sys.exit (0)
		
	def on_selection_changed (self, *args):
		self.remove.set_sensitive (self.masterer.count_selected() > 0)
		
	def on_contents_changed (self, *args):
		is_sensitive = len(self.masterer.source) > 0
		self.clear.set_sensitive (is_sensitive)
		# Only set it sentitive if the drive is available and is not recording
		if self.preferences.drive is not None and not self.recording:
			self.burn.set_sensitive (is_sensitive)

	def remove_file (self, *args):
		self.masterer.remove_selected()
		
	def clear_files (self, *args):
		self.masterer.source.clear()
		
	def add_file (self, *args):
		if self.__add_file.run () == gtk.RESPONSE_OK:
			files = self.__add_file.get_uris()
			hints_list = []
			for uri in files:
				hints_list.append({'location': uri})
			self.masterer.add_files (hints_list)
		self.__add_file.unselect_all()
		self.__add_file.hide()
	
	def __on_preferences (self, *args):
		self.preferences.dialog.run ()
		self.preferences.dialog.hide ()
	
	def __on_about (self, widget, *args):
		a = gtk.AboutDialog ()
		a.set_name ("Serpentine")
		a.set_version (self.preferences.version)
		a.set_website ("http://s1x.homelinux.net/projects/serpentine")
		a.set_copyright ("2004-2005 Tiago Cogumbreiro")
		a.set_transient_for (self)
		a.run ()
		a.hide()
	
	def __on_about_closed (self, about, widget):
		widget.set_sensitive (True)
	
	# This method is associated with the listener to the recording operation
	def on_finished (self, event):
		dev = event.source.drive.get_device ()
		assert dev in self.__recording, (dev, self.__recording)
		self.__recording.remove (dev)
		self.on_contents_changed ()

class CacheError (StandardError):
	CACHE_INVALID = 1
	CACHE_NO_SPACE = 2
	def __init__ (self, error):
		self.__error_id = error
	
	error_id = property (lambda self: self.__error_id)

def __hig_bytes (bytes):
	hig_desc = [("GByte", "GBytes"),
	            ("MByte", "MBytes"),
	            ("KByte", "KByte" ),
	            ("byte" , "bytes" )]
	value, strings = __decompose_bytes (bytes, 30, hig_desc)
	return "%.1f %s" % (value, __plural (value, strings))

def __decompose_bytes (bytes, offset, hig_desc):
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
		return __decompose_bytes (bytes, offset - 10, hig_desc)

def __plural (value, strings):
	if value == 1:
		return strings[0]
	else:
		return strings[1]

def record_musiclist (musiclist, parent = None, preferences = None):
	clear_pool_on_exit = False
	if preferences is None:
		preferences = RecordingPreferences (DATA_DIR)
		clear_pool_on_exit = True
		
	if not preferences.temporary_dir_is_ok():
		gtkutil.dialog_warn ("Cache directory location unavailable", "Please check if the cache location exists and has writable permissions.", parent)
		raise CacheError
		
	# Check if we have space available in our cache dir
	secs = 0
	for music in musiclist:
		if not preferences.pool.is_available (music["location"]):
			secs += music['duration']
	# 44100hz * 16bit * 2channels / 8bits = 176400 bytes per sec
	size_needed = secs * 176400L
	try:
		s = os.statvfs (preferences.temporary_dir)
	except OSError:
		gtkutil.dialog_warn ("Cache directory location unavailable", "Please check if the cache location exists and has writable permissions.", parent)
		raise CacheError
		
	size_avail = s[statvfs.F_BAVAIL] * long(s[statvfs.F_BSIZE])
	if (size_avail - size_needed) < 0:
		
		gtkutil.dialog_error ("Not enough space on cache directory",
			"Remove some music tracks or make sure your cache location location has enough free space (about %s)." % __hig_bytes(size_needed - size_avail) , parent)
		raise CacheError
	
	r = RecordingMedia (musiclist, preferences, parent)
	r.start()
	if clear_pool_on_exit:
		preferences.pool.clear()

gobject.type_register (Serpentine)

if __name__ == '__main__':
	s = Serpentine ()
	s.preferences.simulate = len(sys.argv) == 2 and sys.argv[1] == '--simulate'
	s.show()
	gtk.main()
