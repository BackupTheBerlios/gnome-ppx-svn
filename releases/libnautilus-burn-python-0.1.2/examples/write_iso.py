import sys
sys.path.append ('..')
import nautilus_burn
import gtk
import gobject

if len(sys.argv) != 2:
	print "Usage: write_iso <iso>"
	print "Writes an ISO in maximum speed in simulation mode."
	sys.exit (1)

def on_progress_changed (recorder, fract):
	print fract
	
r = burn.CdRecorder()
cd, = burn.scan_for_cdroms(True)
t = burn.DataTrack()
t.filename = sys.argv[1]
r.connect ('progress-changed', on_progress_changed)
r.write_tracks (cd, [t], cd.get_max_speed_write(), burn.RECORDER_WRITE_DUMMY_WRITE)
print "done"
