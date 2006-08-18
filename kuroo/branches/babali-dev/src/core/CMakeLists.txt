INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/config ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/history ${CMAKE_SOURCE_DIR}/src/intro ${CMAKE_SOURCE_DIR}/src/logs ${CMAKE_SOURCE_DIR}/src/portage ${CMAKE_SOURCE_DIR}/src/queue ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(core_STAT_SRCS
scanhistoryjob.h scanupdatesjob.h scanportagejob.h signalist.h
		threadweaver.h portagedb.h emerge.h cacheportagejob.h etcupdate.h packageitem.h
		tooltip.h packagelistview.h categorieslistview.h packageversion.h dependatom.h
		portagefiles.h images.h packageinspector.h versionview.h filewatcher.h global.h
	dependencyview.h
	scanupdatesjob.cpp scanhistoryjob.cpp emerge.cpp
		scanportagejob.cpp signalist.cpp threadweaver.cpp portagedb.cpp cacheportagejob.cpp
		etcupdate.cpp packageitem.cpp tooltip.cpp packagelistview.cpp categorieslistview.cpp
		packageversion.cpp dependatom.cpp portagefiles.cpp images.cpp
		packageinspector.cpp versionview.cpp filewatcher.cpp global.cpp dependencyview.cpp
)

SET( core_UI
inspectorbase.ui
)

KDE3_ADD_UI_FILES(core_STAT_SRCS ${core_UI})

KDE3_AUTOMOC(${core_STAT_SRCS})
ADD_LIBRARY(core STATIC ${core_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
#	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
#	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
#	-I$(top_builddir)/src/config $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_HEADERS = scanhistoryjob.h scanupdatesjob.h scanportagejob.h signalist.h \
#		threadweaver.h portagedb.h emerge.h cacheportagejob.h etcupdate.h packageitem.h \
#		tooltip.h packagelistview.h categorieslistview.h packageversion.h dependatom.h \
#		portagefiles.h images.h packageinspector.h versionview.h filewatcher.h global.h \
#	dependencyview.h
#
#libcore_la_SOURCES = scanupdatesjob.cpp scanhistoryjob.cpp emerge.cpp \
#		scanportagejob.cpp signalist.cpp threadweaver.cpp portagedb.cpp cacheportagejob.cpp \
#		etcupdate.cpp packageitem.cpp tooltip.cpp packagelistview.cpp categorieslistview.cpp \
#		packageversion.cpp dependatom.cpp portagefiles.cpp images.cpp inspectorbase.ui \
#		packageinspector.cpp versionview.cpp filewatcher.cpp global.cpp dependencyview.cpp
#
#noinst_LTLIBRARIES = libcore.la
#
