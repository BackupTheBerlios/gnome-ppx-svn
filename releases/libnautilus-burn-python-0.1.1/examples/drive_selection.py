import sys
# load module beneath
sys.path.append ('..')
import gtk, gobject
import nautilus_burn

s = nautilus_burn.DriveSelection()

def foo ():
	global s;
	print s.get_device()


s.show()

b = gtk.VBox(False, 0)
b.show()

w = gtk.Window (gtk.WINDOW_TOPLEVEL)
w.add (b)
w.show()

b.add (s)
s.set_property ('show-recorders-only', True)
gobject.idle_add (foo)
gtk.main ()
