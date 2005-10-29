INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/installed \
	-I$(top_srcdir)/src/intro -I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage \
	-I$(top_srcdir)/src/queue -I$(top_srcdir)/src/results -I$(top_srcdir)/src/updates \
	-I$(top_builddir)/src/core -I$(top_builddir)/src/config $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libupdates.la

libupdates_la_SOURCES = updatelistview.cpp updatepackageslistview.cpp \
	updatesbase.ui updatestab.cpp updates.cpp

noinst_HEADERS = updatelistview.h updatepackageslistview.h updatestab.h \
	updates.h

