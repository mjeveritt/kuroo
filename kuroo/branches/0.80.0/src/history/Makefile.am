INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core  -I$(top_srcdir)/src/intro \
	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
	-I$(top_srcdir)/src/results -I$(top_srcdir)/src/history \
	-I$(top_builddir)/src/intro -I$(top_builddir)/src/core -I$(top_builddir)/src/config \
	 -I$(top_builddir)/src/portage  \
	-I$(top_builddir)/src/queue -I$(top_builddir)/src/results -I$(top_builddir)/src/logs \
	-I$(top_builddir)/src/core $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libhistory.la

noinst_HEADERS = history.h historylistview.h historytab.h mergelistview.h \
	mergetab.h

libhistory_la_SOURCES = history.cpp historylistview.cpp historytab.cpp \
			historybase.ui mergelistview.cpp mergebase.ui mergetab.cpp
