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

import dbus, os.path, os, time
from subprocess import Popen
import constants
SERVICE_NAME = "de.berlios.Serpentine"
DOMAIN = "/de/berlios/Serpentine"

class Application (dbus.Object):
	"""Provides services for Serpentine itself."""
	def __init__ (self, service, serpentine_object):
		dbus.Object.__init__ (self, DOMAIN + "/Application", service, [
			self.Quit,
			self.Record,
			self.GetFiles
		])
		self.__serp = serpentine_object
		self.__pid = None
	
	serpentine = property (lambda self: self.__serp)
	
	def Quit (self, message):
		self.serpentine.quit ()
	
	def Record (self, message):
		self.serpentine.burn ()
	
	def GetFiles (self, message):
		filenames = []
		for music in self.serpentine.masterer:
			filenames.append (music['location'])
		return filenames
	
	def GetFileTitle (self, message, location):
		for music in self.serpentine.masterer:
			if music['location'] == location:
				return music['title']
		return None
	
	def GetFileArtist (self, message, location):
		for music in self.serpentine.masterer:
			if music['location'] == location:
				return music['artist']
		return None
	
	def RecordFiles (self, message, files):
		pass
	


def __start_service (bus):
	"""Private function for starting the Serpentine service"""
	# First we try to find out if the service has already started
	dbus_srv = bus.get_service ("org.freedesktop.DBus")
	dbus_obj = dbus_srv.get_object ("/org/freedesktop/DBus", "org.freedesktop.DBus")
	if SERVICE_NAME not in dbus_obj.ListServices ():
		proc = Popen ([os.path.join (constants.bin_dir, "serpentine-service")])
		print "Launching serpentine-service daemon."
		while SERVICE_NAME not in dbus_obj.ListServices ():
			# Check if process is still alive
			# TODO catch OSError
			os.kill (proc.pid, 0)
			time.sleep (0.1)
		print "Serpentine service started with pid", proc.pid
			

def __get_dbus_object (name, domain, service_name):
	"""Private helper function to return objects with a certain standard."""
	bus = dbus.SessionBus()
	__start_service (bus)
	service = bus.get_service (service_name)
	return service.get_object ("%s/%s" % (domain, name), "%s.%s" % (service_name, name))

def get_drives_manager ():
	"""Returns the drives manager, which takes care of Drives' allocation and
	deallocation, providing a way for serpentine to be aware of exclusivity of
	drives."""
	return __get_dbus_object ("DrivesManager", DOMAIN, SERVICE_NAME)
	
def get_application ():
	"""Provides an interface to access common used methods of serpentine."""
	return __get_dbus_object ("Application", DOMAIN, SERVICE_NAME)

