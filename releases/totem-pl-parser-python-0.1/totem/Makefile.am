INCLUDES = -I$(TOTEM_PL_PARSER_CFLAGS)

EXTRA_DIST = totemplparser-marshal.list

noinst_LTLIBRARIES = libtotem_pl_parser.la

libtotem_pl_parser_la_SOURCES =    \
	totem-pl-parser-builtins.h \
	totem-pl-parser-builtins.c \
	totemplparser-marshal.c    \
	totemplparser-marshal.h    \
	totem-disc.h               \
	totem-disc.c               \
	totem-pl-parser.h          \
	totem-pl-parser.c

libtotem_pl_parser_la_LIBADD = $(TOTEM_PL_PARSER_LIBS)

totemplparser-marshal.c: totemplparser-marshal.h
	 ( $(GLIB_GENMARSHAL) --prefix=totemplparser_marshal $(srcdir)/totemplparser-marshal.list --header --body > totemplparser-marshal.c )
totemplparser-marshal.h: totemplparser-marshal.list
	( $(GLIB_GENMARSHAL) --prefix=totemplparser_marshal $(srcdir)/totemplparser-marshal.list --header > totemplparser-marshal.h )

totem-pl-parser-builtins.h: stamp-totem-pl-parser-builtins.h
	@true
	
stamp-totem-pl-parser-builtins.h: totem-pl-parser.h Makefile
	(cd $(srcdir) && $(GLIB_MKENUMS) \
			--fhead "#ifndef __TOTEM_PL_PARSER_BUILTINS_H__\n#define __TOTEM_PL_PARSER_BUILTINS_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" \
			--fprod "/* enumerations from \"@filename@\" */\n" \
			--vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n#define TOTEM_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
			--ftail "G_END_DECLS\n\n#endif /* __TOTEM_PL_PARSER_BUILTINS_H__ */" totem-pl-parser.h) >> xgen-gtbh \
	&& (cmp -s xgen-gtbh totem-pl-parser-builtins.h || cp xgen-gtbh totem-pl-parser-builtins.h ) \
	&& rm -f xgen-gtbh \
	&& echo timestamp > $(@F)
	
totem-pl-parser-builtins.c: totem-pl-parser.h Makefile totem-pl-parser-builtins.h
	(cd $(srcdir) && $(GLIB_MKENUMS) \
			--fhead "#include \"totem-pl-parser.h\"" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {" \
			--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n" \
		totem-pl-parser.h ) > xgen-gtbc \
	&& cp xgen-gtbc totem-pl-parser-builtins.c  \
	&& rm -f xgen-gtbc

CLEANFILES = \
	totem-pl-parser-builtins.h \
	totem-pl-parser-builtins.c \
	totemplparser-marshal.c    \
	totemplparser-marshal.h    \
	stamp-totem-pl-parser-builtins.h
