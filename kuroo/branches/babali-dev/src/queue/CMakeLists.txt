INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/config ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/history ${CMAKE_SOURCE_DIR}/src/intro ${CMAKE_SOURCE_DIR}/src/logs ${CMAKE_SOURCE_DIR}/src/portage ${CMAKE_SOURCE_DIR}/src/queue ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(queue_STAT_SRCS
queuetab.cpp
queue.cpp
queuelistview.cpp
)

KDE3_AUTOMOC(${queue_STAT_SRCS})

SET( queue_UI
queuebase.ui
)

KDE3_ADD_UI_FILES(queue_STAT_SRCS ${queue_UI} )

ADD_LIBRARY(queue STATIC ${queue_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
#	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
#	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
#	-I$(top_builddir)/src/core -I$(top_builddir)/src/config $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libqueue.la
#
#libqueue_la_SOURCES = queuebase.ui queuetab.cpp queue.cpp queuelistview.cpp
#
#noinst_HEADERS = queuetab.h queue.h queuelistview.h
#
#libqueue_la_LIBADD = $(top_builddir)/src/core/libcore.la
