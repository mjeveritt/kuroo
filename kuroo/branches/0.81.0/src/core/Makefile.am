INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
	-I$(top_builddir)/src/config $(all_includes) $(PYTHONINC)

METASOURCES = AUTO

noinst_HEADERS = scanhistoryjob.h scanupdatesjob.h signalist.h threadweaver.h \
	portagedb.h emerge.h etcupdate.h packageitem.h tooltip.h packagelistview.h \
	categorieslistview.h packageversion.h dependatom.h portagefiles.h images.h \
	packageinspector.h versionview.h filewatcher.h global.h loadportagejob.h pythonizer.h

libcore_la_SOURCES = scanupdatesjob.cpp scanhistoryjob.cpp emerge.cpp \
	signalist.cpp threadweaver.cpp portagedb.cpp etcupdate.cpp packageitem.cpp \
	tooltip.cpp packagelistview.cpp categorieslistview.cpp packageversion.cpp \
	dependatom.cpp portagefiles.cpp images.cpp inspectorbase.ui packageinspector.cpp \
	versionview.cpp filewatcher.cpp global.cpp loadportagejob.cpp pythonizer.cpp

noinst_LTLIBRARIES = libcore.la

libcore_la_LIBDFLAGS = $(PYTHONLIB)
libcore_la_LIBDADD = $(LIBPYTHON)
