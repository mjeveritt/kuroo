INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
        -I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
        -I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
        -I$(top_builddir)/src/ -I$(top_builddir)/src/queue -I$(top_builddir)/src/intro \
        -I$(top_builddir)/src/history -I$(top_builddir)/src/core -I$(top_builddir)/src/config \
        -I$(top_builddir)/src/portage -I$(top_builddir)/src/logs -I$(top_builddir)/src/datastructs $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libportage.la

libportage_la_SOURCES = portagebase.ui portagelistview.cpp portagetab.cpp \
	portage.cpp uninstallinspector.cpp uninstallbase.ui

noinst_HEADERS = portagelistview.h portagetab.h portage.h uninstallinspector.h

libportage_la_LIBADD = $(top_builddir)/src/core/libcore.la
