INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history \
	-I$(top_srcdir)/src/intro -I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage \
	-I$(top_srcdir)/src/queue -I$(top_srcdir)/src/results \
	-I$(top_builddir)/src/core -I$(top_builddir)/src/config $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libportage.la

libportage_la_SOURCES = portagebase.ui portagelistview.cpp portagetab.cpp \
	search.ui portage.cpp packageinspector.cpp installed.cpp updates.cpp \
	inspectorbase.ui

noinst_HEADERS = portagelistview.h portagetab.h portage.h packageinspector.h \
	installed.h updates.h

libportage_la_LIBADD = $(top_builddir)/src/core/libcore.la \
	$(top_builddir)/src/queue/libqueue.la
