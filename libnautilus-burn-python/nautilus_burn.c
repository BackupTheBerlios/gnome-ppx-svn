/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 3 "nautilus_burn.override"
#define NO_IMPORT_PYGOBJECT
#include "pygobject.h"
#include <cd-drive.h>
#include <bacon-cd-selection.h>
#include <cd-recorder.h>
#define BACON_TYPE_CD_SELECTION (bacon_cd_selection_get_type())
#include "nb_typebuiltins.h"
#include "nb_drive.h"
/*#include "nb_track.h"*/
#line 18 "nautilus_burn.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGtkComboBox_Type;
#define PyGtkComboBox_Type (*_PyGtkComboBox_Type)
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject PyBaconCdSelection_Type;
PyTypeObject PyCDRecorder_Type;


/* ----------- BaconCdSelection ----------- */

#line 22 "nautilus_burn.override"
static int
_wrap_bacon_cd_selection_new(PyGObject *self)
{
    self->obj = (GObject *)bacon_cd_selection_new();

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create DriveSelection object.");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

#line 49 "nautilus_burn.c"


static PyObject *
_wrap_bacon_cd_selection_set_device(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "device", NULL };
    char *device;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:BaconCdSelection.set_device", kwlist, &device))
        return NULL;
    bacon_cd_selection_set_device(BACON_CD_SELECTION(self->obj), device);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_bacon_cd_selection_get_device(PyGObject *self)
{
    const gchar *ret;

    ret = bacon_cd_selection_get_device(BACON_CD_SELECTION(self->obj));
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_bacon_cd_selection_get_default_device(PyGObject *self)
{
    const gchar *ret;

    ret = bacon_cd_selection_get_default_device(BACON_CD_SELECTION(self->obj));
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 37 "nautilus_burn.override"
static PyObject *
_wrap_bacon_cd_selection_get_cdrom (PyGObject *self)
{
	const CDDrive *drive;
	drive = bacon_cd_selection_get_cdrom ((BaconCdSelection*)self->obj);
	if (drive == NULL) {
		Py_INCREF (Py_None);
		return Py_None;
	}
	return nb_drive_new_from_native (drive);
}
#line 101 "nautilus_burn.c"


static PyMethodDef _PyBaconCdSelection_methods[] = {
    { "set_device", (PyCFunction)_wrap_bacon_cd_selection_set_device, METH_VARARGS|METH_KEYWORDS },
    { "get_device", (PyCFunction)_wrap_bacon_cd_selection_get_device, METH_NOARGS },
    { "get_default_device", (PyCFunction)_wrap_bacon_cd_selection_get_default_device, METH_NOARGS },
    { "get_drive", (PyCFunction)_wrap_bacon_cd_selection_get_cdrom, METH_NOARGS },
    { NULL, NULL, 0 }
};

PyTypeObject PyBaconCdSelection_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "burn.DriveSelection",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,			/* tp_getattro */
    (setattrofunc)0,			/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyBaconCdSelection_methods,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_bacon_cd_selection_new,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- CDRecorder ----------- */

#line 152 "nautilus_burn.override"
static int
_wrap_cd_recorder_new (PyGObject *self)
{
    self->obj = (GObject *)cd_recorder_new();

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create Recorder object.");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

#line 175 "nautilus_burn.c"


#line 62 "nautilus_burn.override"

static int
nb_Object_CheckFromString (PyObject *obj, char *module_name, char *class_name)
{
	int ret;
	PyObject *module, *klass;
	module = PyImport_ImportModule (module_name);
	if (!module)
		return 0;
	klass = PyObject_GetAttrString (module, class_name);
	if (!klass)
		return 0;
	ret = PyObject_IsInstance (obj, klass);
	Py_DECREF (klass);
	Py_DECREF (module);
	return ret;
}

static Track *
nb_track_new_from_object (PyObject *obj)
{
	Track *t = g_new (Track, 1);
	PyObject *data;
	
	if (!t)
		return NULL;

	if (nb_Object_CheckFromString (obj, "nautilus_burn", "AudioTrack")) {
		t->type = TRACK_TYPE_AUDIO;
		data = PyObject_GetAttrString (obj, "filename");
		if (!data || !PyString_Check (data))
			return NULL;
		t->contents.audio.filename = g_strdup (PyString_AS_STRING (data));
		Py_DECREF (data);
		
		data = PyObject_GetAttrString (obj, "cdtext");
		if (!data || !PyString_Check (data))
			return NULL;
		t->contents.audio.cdtext = g_strdup (PyString_AS_STRING (data));
		Py_DECREF (data);
		return t;
		
	} else if (nb_Object_CheckFromString (obj, "nautilus_burn", "DataTrack")) {
		t->type = TRACK_TYPE_DATA;
		data = PyObject_GetAttrString (obj, "filename");
		if (!data || !PyString_Check (data))
			return NULL;
		t->contents.data.filename = g_strdup (PyString_AS_STRING (data));
		Py_DECREF (data);
		return t;
	}
	
	return NULL;
}

static PyObject *
_wrap_cd_recorder_write_tracks (PyGObject *self, PyObject *args, PyObject *kwargs)
{
	nb_Drive *drive;
	PyObject *tracks;
	Track *t;
	GList *g_tracks = NULL;
	PyGILState_STATE gstate;
	
	int speed, flags, i, len, ret;

	if (!PyArg_ParseTuple (args, "OOii", &drive, &tracks, &speed, &flags))
		return NULL;
	if (!PyList_Check (tracks))
		return NULL;
		
	len = PyList_GET_SIZE (tracks);
	for (i = 0; i < len; i++) {
		t = nb_track_new_from_object (PyList_GET_ITEM (tracks, i));
		if (!t)
			return NULL;
		g_tracks = g_list_append (g_tracks, t);
	}
	gstate = PyGILState_Ensure();
	ret = cd_recorder_write_tracks ((CDRecorder*)self->obj, drive->drive, g_tracks, speed, flags);
	PyGILState_Release(gstate);
	
	g_list_foreach (g_tracks, (GFunc)cd_recorder_track_free, NULL);
	g_list_free (g_tracks);
	
	return Py_BuildValue ("i", ret);
}

#line 267 "nautilus_burn.c"


#line 50 "nautilus_burn.override"
static PyObject *
_wrap_cd_recorder_cancel (PyGObject *self, PyObject *args)
{
	int skip_if_dangerous = TRUE;
	
	if (!PyArg_ParseTuple(args, "|i:Recorder.cancel", &skip_if_dangerous))
		return NULL;
	
	return Py_BuildValue ("i", cd_recorder_cancel ((CDRecorder *)self->obj, skip_if_dangerous));
}
#line 281 "nautilus_burn.c"


static PyObject *
_wrap_cd_recorder_get_error_message(PyGObject *self)
{
    const gchar *ret;

    ret = cd_recorder_get_error_message(CD_RECORDER(self->obj));
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_cd_recorder_get_error_message_details(PyGObject *self)
{
    const gchar *ret;

    ret = cd_recorder_get_error_message_details(CD_RECORDER(self->obj));
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef _PyCDRecorder_methods[] = {
    { "write_tracks", (PyCFunction)_wrap_cd_recorder_write_tracks, METH_VARARGS },
    { "cancel", (PyCFunction)_wrap_cd_recorder_cancel, METH_VARARGS },
    { "get_error_message", (PyCFunction)_wrap_cd_recorder_get_error_message, METH_NOARGS },
    { "get_error_message_details", (PyCFunction)_wrap_cd_recorder_get_error_message_details, METH_NOARGS },
    { NULL, NULL, 0 }
};

PyTypeObject PyCDRecorder_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "burn.Recorder",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,			/* tp_getattro */
    (setattrofunc)0,			/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyCDRecorder_methods,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_cd_recorder_new,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- functions ----------- */

PyMethodDef nautilus_burn_functions[] = {
    { NULL, NULL, 0 }
};


/* ----------- enums and flags ----------- */

void
nautilus_burn_add_constants(PyObject *module, const gchar *strip_prefix)
{
  pyg_flags_add(module, "RecorderWriteFlags", strip_prefix, CD_TYPE_RECORDER_WRITE_FLAGS);
  pyg_enum_add(module, "CdRecorderActions", strip_prefix, CD_TYPE_RECORDER_ACTIONS);
  pyg_enum_add(module, "CdRecorderMedia", strip_prefix, CD_TYPE_RECORDER_MEDIA);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
nautilus_burn_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("gobject")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGObject_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "GObject");
        if (_PyGObject_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name GObject from gobject");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gobject");
        return;
    }
    if ((module = PyImport_ImportModule("gtk")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGtkComboBox_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "ComboBox");
        if (_PyGtkComboBox_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name ComboBox from gtk");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gtk");
        return;
    }


#line 419 "nautilus_burn.c"
    pygobject_register_class(d, "BaconCdSelection", BACON_TYPE_CD_SELECTION, &PyBaconCdSelection_Type, Py_BuildValue("(O)", &PyGtkComboBox_Type));
    pygobject_register_class(d, "CDRecorder", CD_TYPE_RECORDER, &PyCDRecorder_Type, Py_BuildValue("(O)", &PyGObject_Type));
}
