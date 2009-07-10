## Makefile.am for kuroo

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kuroo

# set the include path for X, qt, KDE and subdirs
INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
	-I$(top_builddir)/src/ -I$(top_builddir)/src/queue -I$(top_builddir)/src/intro \
	-I$(top_builddir)/src/history -I$(top_builddir)/src/core -I$(top_builddir)/src/config \
	-I$(top_builddir)/src/portage -I$(top_builddir)/src/logs $(all_includes)

SUBDIRS = config intro core history logs portage queue


# makes pot translation files
messages: rc.cpp
	$(EXTRACTRC) `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` > rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) $$LIST -o $(podir)/kuroo.pot; \
	fi

# this is where the kdelnk file will go 
kdelnkdir = $(kde_appsdir)/System
kdelnk_DATA = kuroo.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/kuroo
rc_DATA = kurooui.rc

# this is to set set autostart of kuroo_watcher
autostartdir = $(datadir)/autostart

# headers for kuroo
noinst_HEADERS = kurooview.h kurooiface.h singleton.h mainwindow.h statusbar.h \
	systemtray.h message.h common.h

# source for kuroo
kuroo_SOURCES = kuroo.cpp kurooiface.skel kurooinit.cpp kurooview.cpp \
	kurooviewbase.ui main.cpp mainwindow.cpp message.cpp messagebase.ui statusbar.cpp \
	systemtray.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

kuroo_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# kuroo links
kuroo_LDADD = $(top_builddir)/src/history/libhistory.la \
	$(top_builddir)/src/config/libconfig.la $(top_builddir)/src/queue/libqueue.la \
	$(top_builddir)/src/portage/libportage.la $(top_builddir)/src/logs/liblogs.la \
	$(top_builddir)/src/intro/libintro.la $(top_builddir)/src/core/libcore.la $(LIB_KFILE) \
	$(LIB_KDECORE) $(SQLITE_LIBS)


