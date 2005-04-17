#pythondir = $(PYTHON_PREFIX)/lib/python$(PYTHON_VERSION)/site-packages
#pkgpythondir = $(pythondir)
#libdir = $(pkgpythondir)/totem

pkgpythondir = $(pyexecdir)/gtk-2.0
pkgpyexecdir = $(pyexecdir)/gtk-2.0
pytotemdir = $(pkgpythondir)/totem
pytotemexecdir = $(pkgpyexecdir)/totem

pytotemexec_LTLIBRARIES = plparser.la
pytotem_PYTHON = __init__.py

INCLUDES = \
	-I$(PL_PARSER_CFLAGS) \
	$(PYTHON_INCLUDES)    \
	-I$(top_srcdir)/totem

plparser_la_SOURCES = pl-parser-module.c playlist_parser.c

plparser_la_LDFLAGS = -module -avoid-version -export-symbols-regex initplparser

plparser_la_LIBADD = \
	$(PL_PARSER_LIBS)   \
	$(top_builddir)/totem/libtotem_pl_parser.la

playlist_parser.c: totem-pl-parser.defs totem-pl-parser.override
	pygtk-codegen-2.0 --prefix playlist_parser            \
		--register $(DEFSDIR)/gdk-types.defs          \
		--register $(DEFSDIR)/gtk-types.defs          \
		--override $(srcdir)/totem-pl-parser.override \
		$(srcdir)/totem-pl-parser.defs > $@

EXTRA_DIST = totem-pl-parser.defs totem-pl-parser.override
