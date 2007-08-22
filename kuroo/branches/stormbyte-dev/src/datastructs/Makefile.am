INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
	-I$(top_builddir)/src/ -I$(top_builddir)/src/queue -I$(top_builddir)/src/intro \
	-I$(top_builddir)/src/history -I$(top_builddir)/src/core -I$(top_builddir)/src/config \
	-I$(top_builddir)/src/portage -I$(top_builddir)/src/logs $(all_includes) \
	-I$(top_builddir)/src/datastructs

METASOURCES = AUTO

noinst_LTLIBRARIES = libdata.la

libdata_la_SOURCES = graph.cpp depgraph.cpp ebuild.cpp

noinst_HEADERS = dlist.h dqueue.h dstack.h ebuild.h graph.h depgraph.h

