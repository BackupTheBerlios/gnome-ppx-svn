#include <Python.h>

#include <nautilus-burn-recorder.h>

typedef struct {
    PyObject_HEAD
    
    NautilusBurnRecorderTrack track;
} nb_Track;

int nb_track_init (PyObject *module);
