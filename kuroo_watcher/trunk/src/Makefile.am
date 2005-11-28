## Makefile.am for kuroo

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kuroo_watcher

# set the include path for X, qt, KDE and subdirs
INCLUDES = $(all_includes)

# embedded sqlite as library
SQLITE_SUBDIR = sqlite
LIB_SQLITE_LOCAL = $(top_builddir)/sqlite/libsqlite.la
sqlite_includes = -I$(top_srcdir)/sqlite

# makes pot translation files
messages: rc.cpp
	$(EXTRACTRC) `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` > rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	$(XGETTEXT) $$LIST -o $(podir)/kuroo_watcher.pot; \
	fi

# this is where the kdelnk file will go 
kdelnkdir = $(kde_appsdir)/System
kdelnk_DATA = kuroo_watcher.desktop

# this is to set set autostart of kuroo_watcher
autostartdir = $(datadir)/autostart
autostart_DATA = kuroo_watcher_autostart.desktop

kde_kcfg_DATA = kuroo_watcher.kcfg

# headers for kuroo_watcher
noinst_HEADERS = gentoowatcher.h gentooagent.h watcher_portagedb.h

# source for kuroo_watcher
kuroo_watcher_SOURCES = gentooagent.cpp gentoowatcher.cpp gentoowatcherbase.ui \
	kuroo_watcher.cpp glsacheckbase.ui gentoowatcher.skel watcher_portagedb.cpp \
	watchersettings.kcfgc watchersettingswidget.ui watchersettingswidget2.ui

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

kuroo_watcher_LDFLAGS = $(KDE_RPATH) $(LIB_KFILE) $(all_libraries)

# kuroo_watcher links
kuroo_watcher_LDADD = $(LIB_KFILE) $(LIB_KDECORE) $(LIB_SQLITE_LOCAL)

