INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
        -I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
        -I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
        -I$(top_builddir)/src/ -I$(top_builddir)/src/queue -I$(top_builddir)/src/intro \
        -I$(top_builddir)/src/history -I$(top_builddir)/src/core -I$(top_builddir)/src/config \
        -I$(top_builddir)/src/portage -I$(top_builddir)/src/logs -I$(top_builddir)/src/datastructs $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = liblogs.la

liblogs_la_SOURCES = log.cpp logsbase.ui logstab.cpp

noinst_HEADERS = log.h logstab.h

liblogs_la_LIBADD = $(top_builddir)/src/history/libhistory.la
