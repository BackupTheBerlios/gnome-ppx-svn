/* 
 * Copyright (C) 2004 Tiago Cogumbreiro <cogumbreiro@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors: Tiago Cogumbreiro <cogumbreiro@users.sf.net>
 */

#include <pygobject.h>
#include <Python.h>
#include "nb_drive.h"
#include <nautilus-burn-recorder.h>
void nautilus_burn_register_classes (PyObject *d);
void nautilus_burn_add_constants(PyObject *module, const gchar *strip_prefix);
extern PyMethodDef nautilus_burn_functions[];

DL_EXPORT(void)
initnautilusburn(void)
{
	PyObject *m, *d;
	
	init_pygobject ();
	
	m = Py_InitModule ("nautilusburn", nautilus_burn_functions);
	nautilus_burn_add_constants (m, "NAUTILUS_BURN_");
#if 0
	/* CDRecorderWriteFlags */
	PyModule_AddIntConstant (m, "RECORDER_WRITE_EJECT", CDRECORDER_EJECT);
	PyModule_AddIntConstant (m, "RECORDER_WRITE_BLANK", CDRECORDER_BLANK);
	PyModule_AddIntConstant (m, "RECORDER_WRITE_DUMMY_WRITE", CDRECORDER_DUMMY_WRITE);
	PyModule_AddIntConstant (m, "RECORDER_WRITE_DISC_AT_ONCE", CDRECORDER_DISC_AT_ONCE);
	PyModule_AddIntConstant (m, "RECORDER_WRITE_DEBUG", CDRECORDER_DEBUG);
	PyModule_AddIntConstant (m, "RECORDER_WRITE_OVERBURN", CDRECORDER_OVERBURN);
	PyModule_AddIntConstant (m, "RECORDER_WRITE_BURNPROOF", CDRECORDER_BURNPROOF);
	/* enum RESULT_* */
	PyModule_AddIntConstant (m, "RECORDER_RESULT_ERROR", RESULT_ERROR);
	PyModule_AddIntConstant (m, "RECORDER_RESULT_CANCEL", RESULT_CANCEL);
	PyModule_AddIntConstant (m, "RECORDER_RESULT_FINISHED", RESULT_FINISHED);
	PyModule_AddIntConstant (m, "RECORDER_RESULT_RETRY", RESULT_RETRY);
	/* CDRecorderMedia */
	PyModule_AddIntConstant (m, "RECORDER_MEDIA_CD", MEDIA_CD);
	PyModule_AddIntConstant (m, "RECORDER_MEDIA_DVD", MEDIA_DVD);
	/* CDRecorderActions */
	PyModule_AddIntConstant (m, "RECORDER_ACTION_PREPARING_WRITE", PREPARING_WRITE);
	PyModule_AddIntConstant (m, "RECORDER_ACTION_WRITING", WRITING);
	PyModule_AddIntConstant (m, "RECORDER_ACTION_FIXATING", FIXATING);
	PyModule_AddIntConstant (m, "RECORDER_ACTION_BLANKING", BLANKING);
#endif
	
	d = PyModule_GetDict (m);

	nautilus_burn_register_classes (d);
	initdrive();
	if (PyErr_Occurred ()) {
		//PyFatalError ("can't initialize module nautilus.burn");
	}


}
