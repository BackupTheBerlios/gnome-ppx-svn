PACKAGES = gtk+-2.0 pygtk-2.0 libnautilus-burn
DEFS = `pkg-config --variable=defsdir pygtk-2.0`
CODEGEN = `pkg-config --variable=codegendir pygtk-2.0`
CFLAGS = `pkg-config --cflags $(PACKAGES)` -I/usr/include/python2.3 -I. -Wall
LDFLAGS = `pkg-config --libs $(PACKAGES)` 
LIBNAUTILUS_DIR = `pkg-config --variable=includedir pygtk-2.0`/libnautilus-burn
LIBNAUTILUS_SOURCES = $(LIBNAUTILUS_DIR)/*.h

all: nautilusburn.so

install:
	python setup.py install

nautilus_burn.c: nautilus_burn.defs nautilus_burn.override
	pygtk-codegen-2.0 --prefix nautilus_burn \
	-r $(DEFS)/gdk-types.defs \
	-r $(DEFS)/gtk-types.defs \
	-o nautilus_burn.override \
	nautilus_burn.defs > $@

nautilusburn.so: nautilus_burn.o nb_module.o nb_typebuiltins.o nb_drive.o nb_track.o
	$(CC) $(LDFLAGS) -shared $^ -o $@

clean:
	rm -f *.o nautilus_burn.pyd nautilus_burn/*.pyc
	rm -rf build
