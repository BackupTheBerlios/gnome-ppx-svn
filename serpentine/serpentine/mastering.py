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

import gtk, gtk.glade, gobject, os.path
import operations, audio
from gtk_util import DictStore
import gtk_util
import gnome.vfs
from operations import OperationsQueue

try:
	import playlist_parser
except ImportError:
	print "No playlist parsers found!"
	playlist_parser = None

################################################################################
# Operations used on AudioMastering
#
# 

class ErrorTrapper (operations.Operation, operations.OperationListener):
	def __init__ (self, parent = None):
		operations.Operation.__init__ (self)
		self.__errors = []
		self.__parent = parent
	
	errors = property (lambda self: self.__errors)
	parent = property (lambda self: self.__parent)
	
	def on_finished (self, event):
		if event.id == operations.ERROR:
			self.errors.append (event.source)
	
	def start (self):
		if len (self.errors) == 0:
			e = operations.FinishedEvent (self, operations.SUCCESSFUL)
			for l in self.listeners:
				l.on_finished (e)
			return
				
		elif len (self.errors) > 1:
			title = "Unsupported file types"
		else:
			title = "Unsupported file type"
			
		filenames = []
		for e in self.errors:
			filenames.append (gnome.vfs.URI(e.uri).short_name)
		del self.__errors
		
		if len (filenames) == 1:
			msg = "The following files were not added:" + "\n"
		else:
			msg = "The following files were not added:" + "\n"
		
		msg +=  " " + filenames[0]
		
		for f in filenames[1:]:
			msg += ", " + f
		gtk_util.dialog_error (title, msg, self.parent)
		
		e = operations.FinishedEvent (self, operations.SUCCESSFUL)
		for l in self.listeners:
			l.on_finished (e)

class AddFile (audio.AudioMetadataListener, operations.Operation):
	# TODO: Implement full Operation here
	
	running = property (lambda self: False)

	def __init__ (self, masterer, uri, insert_at = None, insert_before = None):
		operations.Operation.__init__ (self)
		self.uri = uri
		self.masterer = masterer
		self.insert_at = insert_at
		self.insert_before = insert_before
	
	def start (self):
		oper = audio.gvfs_audio_metadata (self.uri)
		oper.listeners.append (self)
		oper.start()
	
	def on_metadata (self, event, metadata):
		# Real implementation, on gobject's main loop
		track_secs = int(metadata['duration']) 
		track_time = "%.2d:%.2d" % (track_secs / 60, track_secs % 60)
		
		row = {
			"uri": self.uri,
			"filename": "",
			"title": gnome.vfs.URI(self.uri[:-4]).short_name or "Unknown",
			"artist": "Unknow Artist",
			"duration": track_secs,
			"time": track_time
		}
		
		del self.uri
		
		if metadata.has_key ('title'):
			row['title'] = metadata['title']
		if metadata.has_key ('artist'):
			row['artist'] = metadata['artist']
			
		if self.insert_at != None:
			if self.insert_before:
				self.masterer.source.insert_before (self.insert_at, row)
			else:
				self.masterer.source.insert_after (self.insert_at, row)
		else:
			self.masterer.source.append (row)
		
	
	def on_finished (self, evt):
		e = operations.FinishedEvent (self, evt.id)
		for l in self.listeners:
			l.on_finished (e)
			

class SetGraphicalUpdate (operations.Operation):
	def __init__ (self, masterer, update):
		operations.Operation.__init__ (self)
		self.__update = update
		self.__masterer = masterer
	
	running = property (lambda self: False)
	
	can_run = property (lambda self: True)
		
	def start (self):
		self.__masterer.update = self.__update
		if self.__update:
			self.__masterer.update_disc_usage()
		e = operations.FinishedEvent (self, operations.SUCCESSFUL)
		for l in self.listeners:
			l.on_finished (e)

################################################################################

class MusicListListener:
	def on_musics_added (self, event, rows):
		pass
	
	def on_musics_removed (self, event, rows):
		pass

class MusicList (operations.Listenable):
	def __getitem__ (self):
		pass
		
	def append_many (self, rows):
		pass
	
	def append (self, row):
		pass
	
	def insert (self, index, row):
		pass
	
	def insert_many (self, index, rows):
		pass
	
	def __len__ (self):
		pass
	
	def __delitem__ (self, index):
		pass
	
	def delete_many (self, indexes):
		pass
	
	def clear (self):
		pass
	
	def has_key (self, key):
		pass
		

class GtkMusicList (MusicList):
	"Takes care of the data source. Supports events and listeners."
	SPEC = (
			# URI is used in converter
			{"name": "uri", "type": gobject.TYPE_STRING},
			# filename is used in recorder
			{"name": "filename", "type": gobject.TYPE_STRING},
			# Remaining items are for the list
			{"name": "duration", "type": gobject.TYPE_INT},
			{"name": "title", "type": gobject.TYPE_STRING},
			{"name": "artist", "type": gobject.TYPE_STRING},
			{"name": "time", "type": gobject.TYPE_STRING})
			
	def __init__ (self):
		operations.Listenable.__init__ (self)
		self.__model = DictStore (*self.SPEC)
		self.__total_duration = 0
		self.__freezed = False
	
	model = property (fget=lambda self: self.__model, doc="Associated ListStore.")
	total_duration = property (fget=lambda self:self.__total_duration, doc="Total disc duration, in seconds.")
	
	def __getitem__ (self, index):
		return self.model.get (index)
	
	def append_many (self, rows):
		self.__freezed = True
		for row in rows:
			self.append (row)
		self.__freezed = False
		
		rows = tuple(rows)
		e = operations.Event(self)
		for l in self.listeners:
			l.on_musics_added (e, rows)
	
	def append (self, row):
		self.model.append (row)
		self.__total_duration += int(row['duration'])
		if not self.__freezed:
			e = operations.Event (self)
			rows = (row,)
			for l in self.listeners:
				l.on_musics_added (e, rows)
	
	def insert_before (self, index, row):
		self.model.insert_before (self.model.get_iter (index), row)
		self.__total_duration += int (row['duration'])
	
	def insert_after (self, index, row):
		self.model.insert_after (self.model.get_iter (index), row)
		self.__total_duration += int (row['duration'])
	
	def __len__ (self):
		return len(self.model)
	
	def __delitem__ (self, index):
		# Copy native row
		row = dict(self[index])
		del self.model[index]
		self.__total_duration -= row['duration']
		rows = (row,)
		if not self.__freezed:
			e = operations.Event (self)
			for l in self.listeners:
				l.on_musics_removed (e, rows)
	
	def delete_many (self, indexes):
		assert isinstance(indexes, list)
		rows = []
		indexes.sort()
		low = indexes[0] - 1
		# Remove duplicate entries
		for i in indexes:
			if low == i:
				indexes.remove (i)
			low = i
		# Now decrement the offsets
		for i in range (len (indexes)):
			indexes[i] -= i
		
		# Remove the elements directly
		for i in indexes:
			# Copy native row
			r = dict(self.model.get(i))
			rows.append(r)
			self.__total_duration -= r['duration']
			del self.model[i]
		
		# Warn the listeners
		rows = tuple(rows)
		e = operations.Event(self)
		for l in self.listeners:
			l.on_musics_removed (e, rows)
		
	def clear (self):
		rows = []
		for row in iter(self.model):
			# Copy each element
			rows.append(dict(row))
		
		self.model.clear()
		self.__total_duration = 0
		
		rows = tuple(rows)
		e = operations.Event (self)
		for l in self.listeners:
			l.on_musics_removed(e, rows)
	

################################################################################
# Audio Mastering widget
#	
import sys, os.path

class AudioMasteringMusicListener (MusicListListener):
	def __init__ (self, audio_mastering):
		self.__master = audio_mastering
		
	def on_musics_added (self, e, rows):
		self.__master.update_disc_usage()
	
	def on_musics_removed (self, e, rows):
		self.__master.update_disc_usage()

class AudioMastering (gtk.VBox, operations.Listenable):
	SIZE_74 = 0
	SIZE_80 = 1
	SIZE_90 = 2

	disk_sizes = [74 * 60, 80 * 60, 90 * 60]
	
	DND_TARGETS = [
		('SERPENTINE_ROW', gtk.TARGET_SAME_WIDGET, 0),
		('text/uri-list', 0, 1),
		('text/plain', 0, 2),
		('STRING', 0, 3),
	]
	def __init__ (self, preferences):
		gtk.VBox.__init__ (self)
		operations.Listenable.__init__ (self)
		self.__disk_size = 74 * 60
		self.queue.abort_on_failure = False
		self.update = True
		self.source = GtkMusicList ()
		self.source.listeners.append (AudioMasteringMusicListener(self))
		gtk.VBox.__init__ (self)
		g = gtk.glade.XML (os.path.join (preferences.data_dir, "serpentine.glade"),
		                   "audio_container")
		self.add (g.get_widget ("audio_container"))
		self.__setup_track_list (g)
		self.__setup_container_misc (g)
	
	def __setup_container_misc (self, g):
		self.__size_list = g.get_widget ("size_list")
		self.__usage_bar = g.get_widget ("usage_bar")
		self.__capacity_exceeded = g.get_widget ("capacity_exceeded")
		
		self.__size_list.connect ("changed", self.__on_size_changed)
		self.__size_list.set_active (AudioMastering.SIZE_74)
	
	def __setup_track_list (self, g):
		lst = g.get_widget ("track_list")
		lst.set_model (self.source.model)
		# Track value is dynamicly calculated
		r = gtk.CellRendererText()
		col = gtk.TreeViewColumn ("Track", r)
		col.set_cell_data_func (r, self.__generate_track)
		
		r = gtk.CellRendererText()
		r.set_property ('editable', True)
		r.connect ('edited', self.__on_title_edited)
		lst.append_column (col)
		col = gtk.TreeViewColumn ("Title", r, text = self.source.model.index_of("title"))
		lst.append_column (col)
		
		r = gtk.CellRendererText()
		r.set_property ('editable', True)
		r.connect ('edited', self.__on_artist_edited)
		col = gtk.TreeViewColumn ("Artist", r, text = self.source.model.index_of("artist"))
		lst.append_column (col)
		r = gtk.CellRendererText()
		col = gtk.TreeViewColumn ("Duration", r, text = self.source.model.index_of("time"))
		lst.append_column (col)
		
		# TreeView Selection
		self.__selection = lst.get_selection()
		self.__selection.connect ("changed", self.__selection_changed)
		self.__selection.set_mode (gtk.SELECTION_MULTIPLE)
		
		# Listen for drag-n-drop events
		lst.set_reorderable (True)
		#XXX pygtk bug here
		lst.enable_model_drag_source (gtk.gdk.BUTTON1_MASK,
		                              AudioMastering.DND_TARGETS,
		                              gtk.gdk.ACTION_DEFAULT |
		                              gtk.gdk.ACTION_MOVE)

		lst.enable_model_drag_dest (AudioMastering.DND_TARGETS,
		                            gtk.gdk.ACTION_DEFAULT |
		                            gtk.gdk.ACTION_MOVE)
		lst.connect ("drag_data_received", self.__on_dnd_drop)
		lst.connect ("drag_data_get", self.__on_dnd_send)
	
	def __generate_track (self, col, renderer, tree_model, treeiter, user_data = None):
		index = tree_model.get_path(treeiter)[0]
		renderer.set_property ('text', index + 1)
	
	def __on_size_changed (self, *args):
		self.disk_size = AudioMastering.disk_sizes[self.__size_list.get_active()]
	
	def __on_title_edited (self, cell, path, new_text, user_data = None):
		self.source[path]["title"] = new_text
	
	def __on_artist_edited (self, cell, path, new_text, user_data = None):
		self.source[path]["artist"] = new_text
	
	def __on_pl_entry (self, parser, uri, title, genre, uris):
		uris.append(uri)
	
	def __on_dnd_drop (self, treeview, context, x, y, selection, info, timestamp, user_data = None):
		data = selection.data
		uris = []
		
		# Insert details
		insert_at = None
		insert_before = None
		drop_info = treeview.get_dest_row_at_pos(x, y)
		if drop_info:
			insert_at, insert_before = drop_info
			insert_before = (insert_before == gtk.TREE_VIEW_DROP_BEFORE or
			                 insert_before == gtk.TREE_VIEW_DROP_INTO_OR_BEFORE)
		del drop_info
		
		if selection.type == 'application/x-rhythmbox-source':
			#TODO: handle rhythmbox playlists
			return
		elif selection.type == 'SERPENTINE_ROW':
			# Private row
			store, path_list = self.__selection.get_selected_rows ()
			if not path_list or len (path_list) != 1:
				return
			path, = path_list
			# Copy the row
			row = dict(self.source[path])
			# Remove old row
			del self.source[path]
			# Append this row
			if insert_at != None:
				if insert_before:
					self.source.insert_before (insert_at, row)
				else:
					self.source.insert_after (insert_at, row)
			else:
				self.source.append (row)
			return
			
		for line in data.split("\n"):
			line = line.strip()
			if len (line) < 1:
				continue
			try:
				nfo = gnome.vfs.get_file_info (line)
				if nfo.type == gnome.vfs.FILE_TYPE_DIRECTORY:
					pass
					# Handle directory importing
				else:
					uris.append (line)
					
				del nfo
				
			except gnome.vfs.NotFoundError, e:
				print "file not found"
				return
		self.add_files (uris, insert_at, insert_before)
	
	def __on_dnd_send (self, widget, context, selection, target_type, timestamp):
		store, path_list = self.__selection.get_selected_rows ()
		assert path_list and len(path_list) == 1
		path, = path_list # unpack the only element
		selection.set (selection.target, 8, self.source[path]['uri'])
	
	def __hig_duration (self, duration):
		hig_duration = ""
		minutes = duration / 60
		if minutes:
			hig_duration = ("%s %s") %(minutes, minutes == 1 and "minute" or "minutes")
		seconds = duration % 60
		if seconds:
			hig_secs = ("%s %s") %(seconds, seconds == 1 and "second" or "seconds")
			hig_duration += (len (hig_duration) and " and ") + hig_secs
		if not len(hig_duration):
			hig_duration = "Empty"
		return hig_duration
	
	def update_disc_usage (self):
		if not self.update:
			return
		if self.source.total_duration > self.disk_size:
			self.__usage_bar.set_fraction (1)
			self.__capacity_exceeded.show ()
		else:
			self.__usage_bar.set_fraction (self.source.total_duration / float (self.disk_size))
			self.__capacity_exceeded.hide ()
		# Flush events so progressbar redrawing gets done
		while gtk.events_pending():
			gtk.main_iteration(True)
		self.__usage_bar.set_text (self.__hig_duration(self.source.total_duration))
		e = operations.Event(self)
		for l in self.listeners:
			l.on_contents_changed (e)
	
	def __selection_changed (self, treeselection):
		e = operations.Event (self)
		for l in self.listeners:
			l.on_selection_changed (e)
	
	def __set_disk_size (self, size):
		assert size in AudioMastering.disk_sizes
		self.__disk_size = size
		self.__size_list.set_active (AudioMastering.disk_sizes.index(size))
		self.update_disc_usage()
		
	disk_size = property (
			lambda self: self.__disk_size,
			__set_disk_size,
			doc = "Represents the disc size, in seconds.")
	
	def add_file (self, uri):
		w = gtk_util.get_root_parent (self)
		assert isinstance(w, gtk.Window)
		uris = self.__add_playlist (uri)
		if uris:
			self.add_files (uris)
			return
			
		trapper = ErrorTrapper (w)
		a = AddFile (self, uri)
		queue = OperationsQueue()
		queue.abort_on_failure = False
		queue.append (a)
		queue.append (trapper)
		queue.start()
	
	def __add_playlist (self, uri):
		mime = gnome.vfs.get_mime_type (uri)
		if mime == "audio/x-mpegurl" or mime == "audio/x-scpls":
			uris = []
			p = playlist_parser.Parser()
			p.connect("entry", self.__on_pl_entry, uris)
			p.parse(uri, False)
			return uris
		return False
		
	# Add a no op when we don't have a paylist parser
	if playlist_parser == None:
		__add_playlist = lambda self, uri: None
		
	def add_files (self, uris, insert_at = None, insert_before = None):
		# Lock graphical updating on each request and
		# only refresh the UI later
		w = gtk_util.get_root_parent (self)
		assert isinstance(w, gtk.Window), type(w)
		trapper = ErrorTrapper (w)
		queue = OperationsQueue()
		queue.abort_on_failure = False
		queue.append (SetGraphicalUpdate (self, False))
		i = 0
		# Convert to an integer if possible
		if insert_at != None and isinstance (insert_at, tuple):
			insert_at, = insert_at
			
		for uri in uris:
			uris = self.__add_playlist (uri)
			if uris:
				self.add_files(uris)
				continue
				
			ins = insert_at
			if insert_at != None:
				ins += i
			a = AddFile (self, uri, ins, insert_before)
			a.listeners.append (trapper)
			queue.append (a)
			i += 1
			
		queue.append (SetGraphicalUpdate (self, True))
		queue.append (trapper)
		queue.start()

	def remove_selected (self):
		store, path_list = self.__selection.get_selected_rows ()
		if not path_list:
			return
		indexes = []
		for p in path_list:
			assert len(p) == 1
			indexes.append(*p)
			
		self.source.delete_many (indexes)
			
	def count_selected (self):
		return self.__selection.count_selected_rows()
	
	
if __name__ == '__main__':
	import sys, os
	win = gtk.Window()
	win.connect ("delete-event", gtk.main_quit)
	w = AudioMastering ()
	w.show()
	win.add (w)
	win.show()
	

	w.add_file (sys.argv[1])
	w.source.clear()
	gtk.main()
