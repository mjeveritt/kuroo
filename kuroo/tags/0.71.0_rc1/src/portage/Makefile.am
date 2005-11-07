INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/installed \
	-I$(top_srcdir)/src/intro -I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage \
	-I$(top_srcdir)/src/queue -I$(top_srcdir)/src/results -I$(top_srcdir)/src/updates \
	-I$(top_builddir)/src/core -I$(top_builddir)/src/config $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libportage.la

libportage_la_SOURCES = portagebase.ui portagelistview.cpp portagetab.cpp \
	search.ui portagepackagesview.cpp portage.cpp usedialog.cpp usebase.ui

noinst_HEADERS = portagelistview.h portagetab.h portagepackagesview.h portage.h \
	usedialog.h

libportage_la_LIBADD = $(top_builddir)/src/core/libcore.la \
	$(top_builddir)/src/queue/libqueue.la
