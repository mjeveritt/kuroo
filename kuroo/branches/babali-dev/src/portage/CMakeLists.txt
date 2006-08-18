INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/config ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/history ${CMAKE_SOURCE_DIR}/src/intro ${CMAKE_SOURCE_DIR}/src/logs ${CMAKE_SOURCE_DIR}/src/portage ${CMAKE_SOURCE_DIR}/src/queue ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(portage_STAT_SRCS
portagelistview.cpp
portagetab.cpp
portage.cpp
uninstallinspector.cpp
)

KDE3_AUTOMOC(${portage_STAT_SRCS})

SET( portage_UI
portagebase.ui
uninstallbase.ui
)

KDE3_ADD_UI_FILES(portage_STAT_SRCS ${portage_UI} )

ADD_LIBRARY(portage STATIC ${portage_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
#	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
#	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
#	-I$(top_builddir)/src/core -I$(top_builddir)/src/config $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libportage.la
#
#libportage_la_SOURCES = portagebase.ui portagelistview.cpp portagetab.cpp \
#	portage.cpp uninstallinspector.cpp uninstallbase.ui
#
#noinst_HEADERS = portagelistview.h portagetab.h portage.h uninstallinspector.h
#
#libportage_la_LIBADD = $(top_builddir)/src/core/libcore.la
