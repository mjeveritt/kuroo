INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
	-I$(top_builddir)/src/core -I$(top_builddir)/src/config $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libqueue.la

libqueue_la_SOURCES = queuebase.ui queuetab.cpp queue.cpp queuelistview.cpp \
	results.cpp

noinst_HEADERS = queuetab.h queue.h queuelistview.h results.h

libqueue_la_LIBADD = $(top_builddir)/src/core/libcore.la
