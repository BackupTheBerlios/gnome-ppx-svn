/* 
   Copyright (C) 2004 Tiago Cogumbreiro <cogumbreiro@users.sf.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Tiago Cogumbreiro <cogumbreiro@users.sf.net>
 */

#include <pygobject.h>
#include <Python.h>
void playlist_parser_register_classes (PyObject *d);
void playlist_parser_add_constants (PyObject *module, const gchar *strip_prefix);
extern PyMethodDef playlist_parser_functions[];

DL_EXPORT(void)
initplaylist_parser (void)
{
	PyObject *m, *d;
	
	init_pygobject ();
	if (PyImport_ImportModule("gnome.vfs") == NULL) {
        PyErr_SetString(PyExc_ImportError,
            "could not import gobject");
        return;
	}
	m = Py_InitModule ("playlist_parser", playlist_parser_functions);
	d = PyModule_GetDict (m);
	
	playlist_parser_register_classes (d);
	playlist_parser_add_constants (m, "TOTEM_PL_");
	
}
