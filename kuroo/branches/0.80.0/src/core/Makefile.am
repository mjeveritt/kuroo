INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/installed \
	-I$(top_srcdir)/src/intro -I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage \
	-I$(top_srcdir)/src/queue -I$(top_srcdir)/src/results -I$(top_srcdir)/src/updates \
	-I$(top_builddir)/src/config $(all_includes)

METASOURCES = AUTO

noinst_HEADERS = scanhistoryjob.h scanupdatesjob.h scaninstalledjob.h \
	scanportagejob.h signalist.h threadweaver.h portagedb.h emerge.h package.h \
	cacheportagejob.h etcupdate.h packageitem.h tooltip.h packagelistview.h \
	categorieslistview.h

libcore_la_SOURCES = scanupdatesjob.cpp scanhistoryjob.cpp emerge.cpp \
	scaninstalledjob.cpp scanportagejob.cpp signalist.cpp threadweaver.cpp portagedb.cpp \
	cacheportagejob.cpp etcupdate.cpp packageitem.cpp tooltip.cpp packagelistview.cpp \
	categorieslistview.cpp

noinst_LTLIBRARIES = libcore.la
