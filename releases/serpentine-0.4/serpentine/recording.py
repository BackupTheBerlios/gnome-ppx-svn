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
import nautilus_burn
from nautilus_burn import AudioTrack
import gtk, gobject
import operations, gtk_util
from converting import FetchMusicList

class RecordingMedia (operations.OperationsQueueListener):
	def __init__ (self, music_list, preferences, parent = None):
		self.__queue = operations.OperationsQueue ()
		self.__queue.listeners.append (self)
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
		if self.preferences.drive.get_media_type () == nautilus_burn.MEDIA_TYPE_CDRW:
			gtk_util.dialog_warn ("CD-RW disk will be erased",
			                      "Please remove your disk if you want to preserve it's contents.",
			                      self.__parent)
		self.__blocked = False
		self.preferences.pool.temporary_dir = self.preferences.temporary_dir
		oper = FetchMusicList(self.__music_list, self.preferences.pool)
		self.__fetching = oper
		self.__queue.append (oper)
		
		oper = RecordMusicList (self.__music_list, self.preferences, self.__parent)
		                        
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
	
	def on_finished (self, evt):
		self.__prog.hide ()
		gobject.source_remove (self.__source)

################################################################################

class RecordMusicList (operations.MeasurableOperation):
	def __init__ (self, music_list, preferences, parent = None):
		operations.MeasurableOperation.__init__(self)
		self.music_list = music_list
		self.__progress = 0.0
		self.__running = False
		self.parent = parent
		self.__recorder = nautilus_burn.Recorder()
		self.__preferences = preferences
	
	progress = property (lambda self: self.__progress)
	running = property (lambda self: self.__running)
	recorder = property (lambda self: self.__recorder)
	preferences = property (lambda self: self.__preferences)
	
	def start (self):
		self.__running = True
		tracks = []
		for m in self.music_list:
			tracks.append (AudioTrack (filename = m['filename']))
		self.recorder.connect ('progress-changed', self.__on_progress)
		self.recorder.connect ('insert-cd-request', self.__insert_cd)
		gobject.idle_add (self.__thread, tracks)
	
	def stop (self):
		# To cancel you have to send False, sending True just checks
		self.recorder.cancel (False)
		
	def __on_progress (self, source, progress):
		self.__progress = progress
	
	def __thread (self, tracks):
		result = self.recorder.write_tracks (self.preferences.drive,
		                                     tracks,
		                                     self.preferences.speed_write,
		                                     self.preferences.write_flags)

		if result == nautilus_burn.RECORDER_RESULT_FINISHED:
			result = operations.SUCCESSFUL
		elif result == nautilus_burn.RECORDER_RESULT_ERROR:
			result = operations.ERROR
		elif result == nautilus_burn.RECORDER_RESULT_CANCEL:
			result == operations.ABORTED
		elif result == nautilus_burn.RECORDER_RESULT_RETRY:
			#TODO: hanlde this
			result == operations.ERROR
			
		e = operations.FinishedEvent(self, result)
		
		for l in self.listeners:
			l.on_finished (e)
		self.__running = False
	
	def __insert_cd (self, rec, reload_media, can_rewrite, busy_cd):
		# messages from nautilus-burner-cd.c
		if busy_cd:
			msg = "Please make sure another application is not using the drive."
			title = "Drive is busy"
		elif not reload_media and can_rewrite:
			msg = "Please put a rewritable or blank disc into the drive."
			title = "Insert rewritable or blank disc"
		elif not reload_media and not can_rewrite:
			msg = "Please put a blank disc into the drive."
			title = "Insert blank disc"
		elif can_rewrite:
			msg = "Please replace the disc in the drive with a rewritable or blank disc."
			title = "Reload rewritable or blank disc"
		else:
			msg = "Please replace the disc in the drive a blank disc."
			title = "Reload blank disc"
		return gtk_util.dialog_ok_cancel (title, msg, self.parent) == gtk.RESPONSE_OK

if __name__ == '__main__':
	import sys, gobject
	class MyListener:
		def on_finished (self, evt):
			gtk.main_quit()
	
	def print_progress (oper):
#		print oper.progress
		return True
	w = gtk.Window(gtk.WINDOW_TOPLEVEL)
	w.add (gtk.Label("---"))
	w.show_all()
	d = nautilus_burn.get_drives_list (False)[0]
	music_lst = [{'filename': sys.argv[1]}]
	r = RecordMusicList (music_lst, d, d.get_max_speed_write(), 0) #nautilus_burn.RECORDER_WRITE_DUMMY_WRITE
	r.listeners.append (MyListener())
	gobject.timeout_add (250, print_progress, r)
	r.start()
	gtk.main()
