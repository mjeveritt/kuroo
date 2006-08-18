INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/config ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/history ${CMAKE_SOURCE_DIR}/src/intro ${CMAKE_SOURCE_DIR}/src/logs ${CMAKE_SOURCE_DIR}/src/portage ${CMAKE_SOURCE_DIR}/src/queue ${CMAKE_SOURCE_DIR}/src/results ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(logs_STAT_SRCS
log.cpp
logstab.cpp
)

KDE3_AUTOMOC(${logs_STAT_SRCS})

SET( logs_UI
logsbase.ui
)

KDE3_ADD_UI_FILES(logs_STAT_SRCS ${logs_UI} )

ADD_LIBRARY(logs STATIC ${logs_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
#	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
#	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
#	-I$(top_srcdir)/src/results -I$(top_builddir)/src/history -I$(top_builddir)/src/core \
#	-I$(top_builddir)/src/config $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = liblogs.la
#
#liblogs_la_SOURCES = log.cpp logsbase.ui logstab.cpp
#
#noinst_HEADERS = log.h logstab.h
#
#liblogs_la_LIBADD = $(top_builddir)/src/history/libhistory.la
