schema_DATA = \
        gnome-pppoe/schemas/apps_gnome-pppoe.schemas \
        gnome-pppoa/schemas/apps_gnome-pppoa.schemas \
        gnome-pptp/schemas/apps_gnome-pptp.schemas


EXTRA_DIST = \
	specs/gnome-ppx.mspec.in \
	specs/metaspec.py        \
	specs/rpm.tpl            \
	specs/tgz.tpl            \
	specs/makepkg            \
	specs/autopackage.tpl    \
	config.h.in              \
	intl.h                   \
	res/network-config.png   \
	intltool-extract.in      \
	intltool-update.in       \
	intltool-merge.in        \
	ac-helpers/binreloc.m4   \
	$(schema_DATA)           \
	mkinstalldirs

noinst_DATA = autopackage/default.apspec gnome-ppx.spec

accountsdir = ${sysconfdir}/ppp/peers
accounts_DATA = gnome-pppoe/account/pppoe gnome-pppoa/account/pppoa gnome-pptp/account/pptp

pixmapsdir = ${prefix}/share/pixmaps
pixmaps_DATA = res/network-config.png

desktop_in_files = gnome-pppoe/pppoe_dialer.desktop gnome-pppoa/pppoa_dialer.desktop gnome-pptp/pptp_dialer.desktop
desktop_files = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

applicationsdir = ${prefix}/share/applications
applications_DATA = $(desktop_files)

schemadir = @GCONF_SCHEMA_FILE_DIR@

if SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schema_DATA)
endif

gentgz: specs/gnome-ppx.mspec
	(cd specs && ./metaspec.py -t tgz -s gnome-ppx.mspec > ../gentgz)
	chmod +x gentgz

autopackage/default.apspec: specs/gnome-ppx.mspec
	[ -d autopackage ] || ([ ! -d autopackage ] && mkdir autopackage)
	(cd specs && ./metaspec.py -t autopackage -s gnome-ppx.mspec > ../autopackage/default.apspec)

gnome-ppx.spec: specs/gnome-ppx.mspec
	(cd specs && ./metaspec.py -t rpm -s gnome-ppx.mspec > ../gnome-ppx.spec)	

SUBDIRS = binreloc tlib libpppd libpppd-gnome gnome-pppoe gnome-pptp gnome-pppoa po

CLEANFILES = *.bak
