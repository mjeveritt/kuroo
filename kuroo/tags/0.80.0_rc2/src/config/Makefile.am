INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
	-I$(top_builddir)/src/core $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libconfig.la

libconfig_la_SOURCES = settings.kcfgc configdialog.cpp options1.ui options2.ui \
	options7.ui

noinst_HEADERS = configdialog.h

kde_kcfg_DATA = kurooconfig.kcfg
