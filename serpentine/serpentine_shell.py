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

import os

import gtk, gtk.glade, gobject, sys

from mastering import AudioMastering
from converting import GvfsMusicPool, FetchMusicList
from recording import RecordMusicList
import operations, nautilus_burn, gtk_util
from preferences import RecordingPreferences

class RecordingMedia (operations.OperationsQueueListener):
	def __init__ (self, music_list, preferences, parent = None):
		self.__queue = operations.OperationsQueue()
		self.__queue.listeners.append (self)
		self.__pool = GvfsMusicPool()
		self.__parent = parent
		self.__prog = gtk_util.HigProgress (parent)
		self.__prog.set_primary_text ("Recording Audio Disc")
		self.__prog.set_secondary_text ("The audio tracks are going to be written to a disc. This operation may take a long time, depending on data size and write speed.")
		self.__prog.connect ('destroy-event', self.__on_prog_destroyed)
		self.__prog.connect ('response', self.__on_response)
		self.__music_list = music_list
		self.__preferences = preferences
	
	preferences = property (lambda self: self.__preferences)
	
	def __on_prog_destroyed (self, *args):
		self.__prog.hide ()
		return False
		
	def start (self):
		self.__blocked = False
		self.__pool.temporary_dir = self.preferences.temporary_dir
		oper = FetchMusicList(self.__music_list, self.__pool)
		self.__fetching = oper
		self.__queue.append (oper)
		
		oper = RecordMusicList (self.__music_list, self.preferences)
		                        
		oper.recorder.connect ('progress-changed', self.__tick)
		oper.recorder.connect ('action-changed', self.__on_action_changed)
		self.__queue.append (oper)
		self.__recording = oper

		self.__queue.start ()
		self.__source = gobject.timeout_add (200, self.__tick)
		if self.__prog.run () == gtk.RESPONSE_CANCEL and self.__queue.can_stop:
			# Disable the cancel button to make sure user 
			# doesn't click it twice
			self.__prog.set_response_sensitive (gtk.RESPONSE_CANCEL, False)
			self.__queue.stop ()
	
	def __tick (self, *args):
		if self.__queue.running:
			self.__prog.set_progress_fraction (self.__queue.progress)
			
		return True
	
	def __on_response (self, dialog, response):
		if self.__blocked and response == gtk.RESPONSE_CANCEL and self.__queue.can_stop:
			self.__prog.set_response_sensitive (gtk.RESPONSE_CANCEL, False)
			self.__queue.stop ()
	
	def __on_action_changed (self, recorder, action, media):
		if action == nautilus_burn.RECORDER_ACTION_PREPARING_WRITE:
			self.__prog.set_sub_progress_text ("Preparing recorder")
		elif action == nautilus_burn.RECORDER_ACTION_WRITING:
			self.__prog.set_sub_progress_text ("Writing media files to disc")
		elif action == nautilus_burn.RECORDER_ACTION_FIXATING:
			self.__prog.set_sub_progress_text ("Fixating disc")
	
	def before_operation_starts (self, evt, oper):
		if oper == self.__fetching:
			self.__prog.set_sub_progress_text ("Preparing media files")
		else:
			self.__blocked = True
			#self.__prog.set_sub_progress_text ("Recording media files")
	
	def on_finished (self, evt):
		self.__prog.hide ()
		gobject.source_remove (self.__source)
	

class MainWindow (gtk.Window):
	def __init__ (self, masterer):
		gtk.Window.__init__ (self, gtk.WINDOW_TOPLEVEL)
		self.masterer = masterer
		self.masterer.listeners.append (self)
		g = gtk.glade.XML ("serpentine.glade", "main_window_container")
		self.add (g.get_widget ("main_window_container"))
		btn = g.get_widget ("add")
		btn.connect ("clicked", self.add_file)
		g.get_widget("burn").connect ("clicked", self.burn)
		self.connect("destroy", self.quit)
		box = self.get_child()
		masterer.show()
		box.add (masterer)
		w = g.get_widget ("mni_preferences")
		w.connect ('activate', self.__on_preferences)
		self.remove_btn = g.get_widget ("remove")
		self.remove_btn.connect ("clicked", self.remove_file)
		self.remove_btn.set_sensitive (False)
		self.clear_btn = g.get_widget ("clear")
		self.burn_btn = g.get_widget ("burn")
		self.burn_btn.set_sensitive (False)
		self.clear_btn.connect ("clicked", self.clear_files)
		self.clear_btn.set_sensitive (False)
		self.__last_path = None
		self.__add_file = gtk.FileChooserDialog (buttons = (gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_OPEN,gtk.RESPONSE_OK))
		self.__add_file.set_select_multiple (True)
		simulate = len(sys.argv) == 2 and sys.argv[1] == '--simulate'
		self.preferences = RecordingPreferences (simulate)
		
	def burn (self, *args):
		gtk_util.dialog_ok_cancel ("Do you want to continue?", 
		                         "You are about to record a media disk, cancelling a writing operation will make you copy unusable.", self)
		r = RecordingMedia (self.masterer.source, self.preferences, self)
		r.start()
		
	def quit (self, *args):
		gtk.main_quit()
		sys.exit (0)
		
	def on_selection_changed (self, *args):
		self.remove_btn.set_sensitive (self.masterer.count_selected() > 0)
		
	def on_contents_changed (self, *args):
		is_sensitive = len(self.masterer.source) > 0
		self.clear_btn.set_sensitive (is_sensitive)
		self.burn_btn.set_sensitive (is_sensitive)

	def remove_file (self, *args):
		self.masterer.remove_selected()
		
	def clear_files (self, *args):
		self.masterer.source.clear()
		
	def add_file (self, *args):
		# XXX: NEW METHOD
		if self.__add_file.run () == gtk.RESPONSE_OK:
			files = self.__add_file.get_uris()
			self.masterer.add_files (files)
		self.__add_file.unselect_all()
		self.__add_file.hide()
	
	def __on_preferences (self, *args):
		self.preferences.dialog.run ()
		self.preferences.dialog.hide ()

if __name__ == '__main__':
	m = MainWindow (AudioMastering())
	m.show()
	gtk.main()
