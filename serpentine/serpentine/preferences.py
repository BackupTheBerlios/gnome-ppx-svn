import gtk.glade, nautilus_burn, gtk, gobject, os, os.path
import gnome_util
from converting import GvfsMusicPool
import gconf
import gaw
import gtk.gdk
try:
	import release
except Exception:
	release = None
	
gconf.client_get_default ().add_dir ("/apps/serpentine", gconf.CLIENT_PRELOAD_NONE)

class RecordingPreferences (object):
	def __init__ (self):
		self.__write_flags = 0
		
		# Sets up data dir and version
		if release:
			self.__version = release.version
			self.__data_dir = release.data_dir
		else:
			self.__version = "testing"
			self.__data_dir = "ui"
		
		# setup ui
		g = gtk.glade.XML (os.path.join(self.data_dir, "serpentine.glade"), "preferences_dialog")
		self.__dialog = g.get_widget ("preferences_dialog")
		self.dialog.connect ('destroy-event', self.__on_destroy)
		
		# Drive selection
		drv = g.get_widget ("drive")
		cmb_drv = nautilus_burn.DriveSelection ()
		cmb_drv.show ()
		self.__drive_selection = cmb_drv
		drv.pack_start (cmb_drv, False, False)
		
		# Speed selection
		self.__speed = gaw.data_spin_button (g.get_widget ("speed"), '/apps/serpentine/write_speed')
		self.__use_max_speed = gaw.data_toggle_button (g.get_widget ("use_max_speed"), '/apps/serpentine/use_max_speed')
		self.__update_speed ()
		self.__speed.sync_widget()
	
		# eject checkbox
		self.__eject = gaw.data_toggle_button (g.get_widget ("eject"), '/apps/serpentine/eject')
		
		# temp
		self.__tmp = gaw.data_entry (g.get_widget ('location_ent'), "/apps/serpentine/temporary_dir")
		if self.__tmp.data == '':
			self.__tmp.data = '/tmp'
			
		self.__tmp.widget.connect ('changed', self.__on_tmp_changed)
		self.__tmp.sync_widget()
		self.dialog.connect ('show', self.__on_tmp_changed)
		g.get_widget ('location_btn').connect ('clicked', self.__on_tmp_choose)
		self.__tmp_dlg = gtk.FileChooserDialog (action = gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER,
		                                        parent = self.dialog,
		                                        buttons = (gtk.STOCK_CANCEL,
		                                                   gtk.RESPONSE_CANCEL,
		                                                   gtk.STOCK_OPEN,
		                                                   gtk.RESPONSE_OK))
		self.__tmp_dlg.set_local_only (True)
		self.__tmp_dlg.set_filename (self.__tmp.data)
		
		# Pool
		self.__pool = GvfsMusicPool ()
		
		# Close button
		self.__close = g.get_widget ('close_btn')
		
	
	def __update_speed (self):
		if not self.drive:
			self.__speed.set_sensitive (False)
			return
			
		speed = self.drive.get_max_speed_write ()
		assert speed > 0
		self.__speed.widget.set_range (1, speed)

		
	def __set_simulate (self, simulate):
		assert isinstance (simulate, bool)
		if simulate:
			self.__write_flags |= nautilus_burn.RECORDER_WRITE_DUMMY_WRITE
		else:
			self.__write_flags &= ~nautilus_burn.RECORDER_WRITE_DUMMY_WRITE
	
	def __get_simulate (self):
		return (self.__write_flags & nautilus_burn.RECORDER_WRITE_DUMMY_WRITE) == nautilus_burn.RECORDER_WRITE_DUMMY_WRITE
	
	simulate = property (__get_simulate, __set_simulate)
	dialog = property (lambda self: self.__dialog)
	version = property (lambda self: self.__version)
	data_dir = property (lambda self: self.__data_dir)
	drive = property (lambda self: self.__drive_selection.get_drive())
	temporary_dir = property (lambda self: self.__tmp.data)
	pool = property (lambda self: self.__pool)
	
	def __get_speed_write (self):
		assert self.drive
		self.__update_speed()
		if self.__use_max_speed.data:
			return self.drive.get_max_speed_write ()
		print self.__speed.data
		return self.__speed.data
		
	speed_write = property (__get_speed_write)
	
	def __get_write_flags (self):
		ret = self.__write_flags
		if self.__eject.data:
			ret |= nautilus_burn.RECORDER_WRITE_EJECT
		return ret
		
	write_flags = property (__get_write_flags)
	
	def temporary_dir_is_ok (self):
		tmp = self.__tmp.data
		is_ok = False
		try:
			is_ok = os.path.isdir (tmp) and os.access (tmp, os.W_OK)
		except OSError, err:
			pass
		return is_ok
	
	def __on_tmp_choose (self, *args):
		if self.__tmp_dlg.run () == gtk.RESPONSE_OK:
			self.__tmp.data = self.__tmp_dlg.get_filename ()
		self.__tmp_dlg.hide ()

	def __on_destroy (self, *args):
		self.dialog.hide ()
		return False

	def __on_tmp_changed (self, *args):
		is_ok = self.temporary_dir_is_ok ()
		if is_ok:
			self.__tmp.widget.modify_base (gtk.STATE_NORMAL, gtk.gdk.color_parse ("#FFF"))
		else:
			self.__tmp.widget.modify_base (gtk.STATE_NORMAL, gtk.gdk.color_parse ("#F88"))
		self.__close.set_sensitive (is_ok)
