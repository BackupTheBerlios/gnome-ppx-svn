#include <glib.h>
#include <cd-drive.h>

int main () {
	GList *cds, *iter;

	cds = scan_for_cdroms (TRUE, TRUE);
	for (iter = g_list_first (cds); iter; iter = iter->next) {
		printf ("%s\n", ((CDDrive *)iter->data)->device);
	}
	g_list_foreach (cds, cd_drive_free, NULL);
	g_list_free (cds);
	return 0;
}
