# Source files
serpentinedir = $(pythondir)/serpentine
serpentine_PYTHON = \
	serpentine/audio.py        \
	serpentine/gaw.py          \
	serpentine/__init__.py     \
	serpentine/preferences.py  \
	serpentine/converting.py   \
	serpentine/gnome_util.py   \
	serpentine/mastering.py    \
	serpentine/recording.py    \
	serpentine/export.py       \
	serpentine/gtk_util.py     \
	serpentine/operations.py   \
	serpentine/release.py

# Launcher
desktopdir = $(datadir)/applications
desktop_DATA = data/serpentine.desktop
#desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
#INTLTOOL_DESKTOP_RULE

# Glade files
uidir= $(datadir)/serpentine
ui_DATA = data/serpentine.glade

# Executable
bin_SCRIPTS = scripts/serpentine

#DISTCLEANFILES =            \
#	intltool-extract    \
#	intltool-merge      \
#	intltool-update     \
#	$(desktop_DATA)

# Files to be packaged
EXTRA_DIST =                \
	$(ui_DATA)          \
	$(bin_SCRIPTS)      \
	$(desktop_DATA)     

SUBDIRS = po
