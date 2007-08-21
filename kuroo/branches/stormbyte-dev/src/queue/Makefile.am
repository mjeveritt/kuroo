INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
	-I$(top_builddir)/src/ -I$(top_builddir)/src/queue -I$(top_builddir)/src/intro \
	-I$(top_builddir)/src/history -I$(top_builddir)/src/core -I$(top_builddir)/src/config \
	-I$(top_builddir)/src/portage -I$(top_builddir)/src/logs -I$(top_builddir)/src/datastructs $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libqueue.la

libqueue_la_SOURCES = queuebase.ui queuetab.cpp queue.cpp queuelistview.cpp

noinst_HEADERS = queuetab.h queue.h queuelistview.h

libqueue_la_LIBADD = $(top_builddir)/src/core/libcore.la
