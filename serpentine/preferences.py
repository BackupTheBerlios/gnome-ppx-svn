import gtk.glade, nautilus_burn, gtk, gobject

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
		cmb_drv.set_device (cmb_drv.get_default_device ())
		self.__drive_selection = cmb_drv
		drv.pack_start (cmb_drv, False, False)
		
		# Speed selection
		self.__speed = g.get_widget ("speed")
		self.__max_speed = g.get_widget ("use_max_speed")
		self.speed_write
	
		# eject checkbox
		w = g.get_widget ("eject")
		w.connect ("toggled", self.__on_eject)
		self.__on_eject (w)
		
		# temp
		self.__tmp = g.get_widget ("location_ent")
	
	def __update_speed (self):
		speed = self.drive.get_max_speed_write ()
		assert speed > 0
		self.__speed.set_range (1, speed)
		#self.__speed.set_value (speed)
		
	def __set_simulate (self, simulate):
		assert isinstance (simulate, bool)
		if simulate:
			self.__write_flags |= nautilus_burn.RECORDER_WRITE_DUMMY_WRITE
		else:
			self.__write_flags &= ~ nautilus_burn.RECORDER_WRITE_DUMMY_WRITE
	
	simulate = property (lambda self: (self.__write_flags & nautilus_burn.RECORDER_WRITE_DUMMY_WRITE) == nautilus_burn.RECORDER_WRITE_DUMMY, __set_simulate)
	
	dialog = property (lambda self: self.__dialog)
	
	drive = property (lambda self: self.__drive_selection.get_drive())
	
	def __on_destroy (self, *args):
		self.dialog.hide ()
		return False
	
	def __get_speed_write (self):
		self.__update_speed()
		if self.__max_speed.get_active ():
			return self.drive.get_max_speed_write ()
		return self.__speed.get_value_as_int ()
		
	speed_write = property (__get_speed_write)
	
	write_flags = property (lambda self: self.__write_flags)
	
	def __on_eject (self, check, *args):
		if check.get_active ():
			self.__write_flags |= nautilus_burn.RECORDER_WRITE_EJECT
		else:
			self.__write_flags &= ~ nautilus_burn.RECORDER_WRITE_EJECT
	
	temporary_dir = property (lambda self: self.__tmp.get_text())
			
	
