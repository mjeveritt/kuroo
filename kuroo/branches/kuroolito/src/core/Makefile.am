INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
	-I$(top_builddir)/src/config $(all_includes)

METASOURCES = AUTO

noinst_HEADERS = scanupdatesjob.h scanportagejob.h signalist.h threadweaver.h \
	portagedb.h cacheportagejob.h packageitem.h tooltip.h packagelistview.h \
	categorieslistview.h packageversion.h dependatom.h portagefiles.h images.h \
	packageinspector.h versionview.h filewatcher.h global.h dependencyview.h

libcore_la_SOURCES = scanupdatesjob.cpp scanportagejob.cpp signalist.cpp \
		threadweaver.cpp portagedb.cpp cacheportagejob.cpp packageitem.cpp tooltip.cpp \
		packagelistview.cpp categorieslistview.cpp packageversion.cpp dependatom.cpp \
		portagefiles.cpp images.cpp inspectorbase.ui packageinspector.cpp versionview.cpp \
		filewatcher.cpp global.cpp dependencyview.cpp emerge.cpp

noinst_LTLIBRARIES = libcore.la

