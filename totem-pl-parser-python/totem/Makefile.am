PACKAGES = gtk+-2.0 pygtk-2.0 libxml-2.0 gnome-vfs-2.0 glib-2.0 gnome-vfs-module-2.0
DEFS = `pkg-config --variable=defsdir pygtk-2.0`
CODEGEN = `pkg-config --variable=codegendir pygtk-2.0`
CFLAGS = `pkg-config --cflags $(PACKAGES)` -I/usr/include/python2.3 -I. -Wall
LDFLAGS = `pkg-config --libs $(PACKAGES)` 
srcdir=.
GLIB_GENMARSHAL = glib-genmarshal

playlist_parser.so: playlist_parser.o totem-pl-parser-builtins.o pl-parser-module.o totem-pl-parser.o totem-disc.o totemplparser-marshal.o
	$(CC) $(LDFLAGS) -shared $^ -o $@

playlist_parser.c: totem-pl-parser.defs totem-pl-parser.override totem-pl-parser-builtins.h totem-pl-parser-builtins.c totemplparser-marshal.c totemplparser-marshal.h
	pygtk-codegen-2.0 --prefix playlist_parser \
	-r $(DEFS)/gdk-types.defs   \
	-r $(DEFS)/gtk-types.defs   \
	-o totem-pl-parser.override \
	totem-pl-parser.defs > $@

totemplparser-marshal.c: totemplparser-marshal.h
	 ( $(GLIB_GENMARSHAL) --prefix=totemplparser_marshal $(srcdir)/totemplparser-marshal.list --header --body > totemplparser-marshal.c )
totemplparser-marshal.h: totemplparser-marshal.list
	( $(GLIB_GENMARSHAL) --prefix=totemplparser_marshal $(srcdir)/totemplparser-marshal.list --header > totemplparser-marshal.h )

totem-pl-parser-builtins.h: stamp-totem-pl-parser-builtins.h
	@true
	
stamp-totem-pl-parser-builtins.h: totem-pl-parser.h Makefile
	(glib-mkenums \
			--fhead "#ifndef __TOTEM_PL_PARSER_BUILTINS_H__\n#define __TOTEM_PL_PARSER_BUILTINS_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" \
			--fprod "/* enumerations from \"@filename@\" */\n" \
			--vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n#define TOTEM_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
			--ftail "G_END_DECLS\n\n#endif /* __TOTEM_PL_PARSER_BUILTINS_H__ */" totem-pl-parser.h) >> xgen-gtbh \
	&& (cmp -s xgen-gtbh totem-pl-parser-builtins.h || cp xgen-gtbh totem-pl-parser-builtins.h ) \
	&& rm -f xgen-gtbh \
	&& echo timestamp > $(@F)
totem-pl-parser-builtins.c: totem-pl-parser.h Makefile
	(glib-mkenums \
			--fhead "#include \"totem-pl-parser.h\"" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {" \
			--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n" \
		totem-pl-parser.h ) > xgen-gtbc \
	&& cp xgen-gtbc totem-pl-parser-builtins.c  \
	&& rm -f xgen-gtbc

clean:
	rm -rf *.so *.o totem-pl-parser-builtins.h totem-pl-parser-builtins.c totemplparser-marshal.c totemplparser-marshal.h playlist_parser.c stamp-totem-pl-parser-builtins.h
