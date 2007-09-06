INCLUDES= $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES=konqsidebar_konqtest1.la

konqsidebar_konqtest1_la_SOURCES = konqtest1.cpp
konqsidebar_konqtest1_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -lkonqsidebarplugin
konqsidebar_konqtest1_la_LIBADD = $(LIB_KPARTS) $(LIB_KFILE)

konqtest1_entry_DATA = konqtest1.desktop
konqtest1_entrydir = $(kde_datadir)/konqsidebartng/entries
