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
		print "trying to abort it"
		self.recorder.cancel ()
		
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
		elif can_rewrite:
			msg = "Please replace the disc in the drive with a rewritable or blank disc."
			title = "Reload rewritable or blank disc"
		else:
			msg = "Please replace the disc in the drive a blank disc."
			title = "Reload blank disc"
		return gtk_util.dialog_ok_cancel (title, msg) == gtk.RESPONSE_OK

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
