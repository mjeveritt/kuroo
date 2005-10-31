INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/installed \
	-I$(top_srcdir)/src/intro -I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage \
	-I$(top_srcdir)/src/queue -I$(top_srcdir)/src/results -I$(top_srcdir)/src/updates \
	-I$(top_builddir)/src/core -I$(top_builddir)/src/config $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libinstalled.la

libinstalled_la_SOURCES = installedbase.ui installedpackageslistview.cpp \
	installedtab.cpp search.ui installedpackagesview.cpp installed.cpp

noinst_HEADERS = installedpackageslistview.h installedtab.h \
	installedpackagesview.h installed.h

libinstalled_la_LIBADD = $(top_builddir)/src/core/libcore.la \
	$(top_builddir)/src/config/libconfig.la $(top_builddir)/src/queue/libqueue.la
