INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
        -I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
        -I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
        -I$(top_builddir)/src/ -I$(top_builddir)/src/queue -I$(top_builddir)/src/intro \
        -I$(top_builddir)/src/history -I$(top_builddir)/src/core -I$(top_builddir)/src/config \
        -I$(top_builddir)/src/portage -I$(top_builddir)/src/logs -I$(top_builddir)/src/datastructs $(all_includes)

METASOURCES = AUTO

noinst_HEADERS = scanhistoryjob.h scanupdatesjob.h scanportagejob.h signalist.h \
		threadweaver.h portagedb.h emerge.h cacheportagejob.h etcupdate.h packageitem.h \
		tooltip.h packagelistview.h categorieslistview.h packageversion.h dependatom.h \
		portagefiles.h images.h packageinspector.h versionview.h filewatcher.h global.h \
	dependencyview.h

libcore_la_SOURCES = scanupdatesjob.cpp scanhistoryjob.cpp emerge.cpp \
		scanportagejob.cpp signalist.cpp threadweaver.cpp portagedb.cpp cacheportagejob.cpp \
		etcupdate.cpp packageitem.cpp tooltip.cpp packagelistview.cpp categorieslistview.cpp \
		packageversion.cpp dependatom.cpp portagefiles.cpp images.cpp inspectorbase.ui \
		packageinspector.cpp versionview.cpp filewatcher.cpp global.cpp dependencyview.cpp

noinst_LTLIBRARIES = libcore.la

