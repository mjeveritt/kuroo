INCLUDES = $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES=konqsidebar_metabar.la

KDE_ICON = AUTO

konqsidebar_metabar_la_SOURCES = metabar.cpp configdialog.cpp linkwidget.cpp \
	metaframe.cpp serviceloader.cpp
konqsidebar_metabar_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
konqsidebar_metabar_la_LIBADD = $(LIB_KFILE) $(LIB_KPARTS) -lkonqsidebarplugin

metabar_entrydir = $(kde_datadir)/konqsidebartng/entries
metabar_adddir = $(kde_datadir)/konqsidebartng/add
metabar_datadir = $(kde_datadir)/metabar
noinst_HEADERS = metabar.h configdialog.h linkwidget.h metaframe.h \
	serviceloader.h
metabar_entry_DATA = metabar.desktop
metabar_add_DATA = metabar_add.desktop
metabar_data_DATA = iconsrc
