import gtk.glade, nautilus_burn, gtk, gobject
import gnome_util
from converting import GvfsMusicPool
import gconf
import gaw

gconf.client_get_default ().add_dir ("/apps/serpentine", gconf.CLIENT_PRELOAD_NONE)

class RecordingPreferences (object):
	def __init__ (self, simulate = False):
		self.__write_flags = 0
		self.simulate = simulate
		
		g = gtk.glade.XML ("serpentine.glade", "preferences_dialog")
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
		self.__tmp = gaw.data_entry (g.get_widget ("location_ent"), "/apps/serpentine/temporary_dir")
		self.__tmp.sync_widget()
		
		# Pool
		self.__pool = GvfsMusicPool ()
	
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
	
	drive = property (lambda self: self.__drive_selection.get_drive())
	
	def __on_destroy (self, *args):
		self.dialog.hide ()
		return False
	
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
	
#	def __on_eject (self, check, *args):
#		if check.get_active ():
#			self.__write_flags |= 
#		else:
#			self.__write_flags &= ~ nautilus_burn.RECORDER_WRITE_EJECT
	
	temporary_dir = property (lambda self: self.__tmp.data)
			
	pool = property (lambda self: self.__pool)
