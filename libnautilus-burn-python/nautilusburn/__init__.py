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

import os
base_path = os.path.dirname (os.path.abspath(__file__)) + '/..'
base_path = os.path.abspath (base_path)
del os
import imp
imp.load_dynamic ("nautilusburn", base_path + "/nautilusburn.so")
del base_path
del imp

class Track (object):
	def __init__ (self):
		raise NotImplementedError

class DataTrack (Track):
	def __init__ (self, filename = ""):
		self.__filename = filename
		
	def __set_filename (self, f):
		self.__filename = f
	
	filename = property (
		lambda self: self.__filename,
		__set_filename,
		doc = "The ISO9660 filename of the data track.")
class AudioTrack (Track):
	def __init__ (self, filename = "", cdtext = ""):
		self.__filename = filename
		self.__cdtext = cdtext
		
	def __set_filename (self, f):
		self.__filename = f
	
	filename = property (
		lambda self: self.__filename,
		__set_filename,
		doc = "The filename of the audio track. The filename can be a WAV or a RAW cdr format.")

	def __set_cdtext (self, t):
		self.__cdtext = t
	
	cdtext = property (
		lambda self: self.__cdtext,
		__set_cdtext,
		doc = "The CD-TEXT associated with this audio track.")
