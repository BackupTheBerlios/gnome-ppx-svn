#!/usr/bin/env python
#
# metaspec.py
# Copyright (C) 2004 Tiago Cogumbreiro <cogumbreiro@users.sf.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#

"""
usage: metaspec {-t|--type} <output_type> {-s|--spec} <metaspec>

Spec Generator

Converts a meta spec into a spec for RPM, Autopackage or Slackware tgz.

Options:
	-t <type>,
	--type=<type>	Defines the generated spec type.
			Available options: autopackage, rpm, tgz


	-s <file>,
	--spec=<file>	Defines the meta spec.
"""


import sys, getopt


class TagProcessor:
	def __init__(self):
		self.start = None
		self.end = None
	def process (self, buffer):
		pass
	def __string__ (self):
		return "<%s %s %s>" % (self.__name__, self.start.strip(), self.end.strip())
class TagExec (TagProcessor):
	"""Interprets the contents of a tag"""
	def process (self, buffer):
		try:
			exec buffer.strip() in globals()
		except:
			print "Syntax error: ", self, "[[[" + buffer + "]]]"
			sys.exit(1)

class TagEval (TagProcessor):
	"""Returns the value of a variable, represented
	by the tag contents."""

	def process (self, buffer):
		buffer = buffer.strip()
		if buffer not in globals().keys():
			raise ValueError, "%s is not a defined variable." % (buffer)
		print globals()[buffer.strip()],

class HyperParser:
	"""An hyper parser considers a stream with a set
	of tags. Each tag has a opening and closing identifier.
	When the hyperparser finds a tag it sends the string
	inside the tag for it to process it."""

	def __init__(self, tags, buffer_size = 2048):
		self.tags = tags
		self.buffer_size = buffer_size
		self.buffer = ''
		
	def fill_buffer (self):
		if not len(self.buffer):
			self.buffer = self.file.read (self.buffer_size)
			if not len(self.buffer):
				return False
		return True
		
	def parse (self, file):
		# Enumeration of states
		self.file = file
		read_data = True
		found = None
		while 1:
			if not self.fill_buffer():
				break
			if read_data:
				instruction = ''
				found = None
				lower_index = self.buffer_size
				
				for tag in self.tags:
					# Find beginig of tag
					index = self.buffer.find(tag.start)
					if index >= 0 and (not found or lower_index > index):
						found = tag
						lower_index = index

				if found:
					self.parse_data (self.buffer[:lower_index])
					self.buffer = self.buffer[lower_index + len(found.start):]
					read_data = False
				
				else:
					self.parse_data (self.buffer)
					self.buffer = ''
					
			else:
				# Find end of tag
				index = self.buffer.find(found.end)
				if index >= 0:
					instruction += self.buffer[:index]
					found.process (instruction)
					read_data = True
					self.buffer = self.buffer[index + len(found.end):]
				else:
					instruction += self.buffer
					self.buffer = ''
				
			# End for		
			
	def parse_data (self, buffer):
		sys.stdout.write (buffer)


def print_sect (sect, pre, post = ""):
	"""Helper function for printing a section. A section is an
	object with two member variables: 'raw' and 'data'. 
	When 'raw' is True 'data' is printed to output stream.
	When it's not then 'pre' is printed to output stream and
	then 'data' is printed.
	"""
	if sect.raw:
		print sect.data
	else:
		print pre + sect.data + post
		
def print_files(pre, post, files):
	"""Useful for printing pre and post strings
	in a list of files (a simple string array)"""
	
	for f in files:
		print pre + f + post

class Section:
	"""Section has two fields a boolean member
	'raw' a string 'data'. It is used for example on
	the 'compile' variable, to specify if user wants
	to do a raw compile command or use the spec's provided
	directive."""
	def __init__(self):
		self.raw = False
		self.data = ""

supported_types = ('rpm', 'autopackage', 'tgz')

dirname = {'bin': 'bin', 'sbin': 'sbin', 'sysconf': 'etc', 'data': 'share', 'libexec': 'libexec'}
prepare = Section()
compile = Section()
install = Section()
install_extra = ""
compile_extra = ""
prepare_extra = ""
header = ""
files = {}
dependencies = []

def usage ():
	print __doc__
	sys.exit(2)

if __name__ == '__main__':

	###########################
	# Validate arguments
	###########################
	if len(sys.argv) < 3:
		usage()

	try:						
		opts, args = getopt.getopt(sys.argv[1:], "ht:s:", ["help=", "type=", "spec="])
	except getopt.GetoptError:		  
		usage()
		sys.exit(2)

	package_type = None
	metaspec = None

	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
		elif opt in ("-s", "--spec"):
			metaspec = arg
		elif opt in ("-t", "--type"):
			package_type = arg

	if not package_type or not metaspec or package_type not in supported_types:
		usage()

	#############################
	# Start the actual program
	#############################
	
	# load the user metaspec
	f = open(metaspec)
	exec f in globals()
	f.close()

	# Parse the template
	tags = []
	
	t = TagExec()
	t.start = "<?"
	t.end = "?>"
	tags.append (t)

	t = TagEval()
	t.start = "@"
	t.end = "@"
	tags.append (t)

	parser = HyperParser (tags)
	template = open (package_type + ".tpl")
	parser.parse (template)
	template.close ()
