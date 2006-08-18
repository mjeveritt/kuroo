INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/config ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/history ${CMAKE_SOURCE_DIR}/src/intro ${CMAKE_SOURCE_DIR}/src/logs ${CMAKE_SOURCE_DIR}/src/portage ${CMAKE_SOURCE_DIR}/src/queue ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(history_STAT_SRCS
history.cpp
historylistview.cpp
historytab.cpp
mergelistview.cpp
mergetab.cpp
)

KDE3_AUTOMOC(${history_STAT_SRCS})

SET( history_UI
historybase.ui
mergebase.ui
)

KDE3_ADD_UI_FILES(history_STAT_SRCS ${history_UI} )

ADD_LIBRARY(history STATIC ${history_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
#	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
#	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
#	-I$(top_builddir)/src/intro -I$(top_builddir)/src/core -I$(top_builddir)/src/config \
#	-I$(top_builddir)/src/portage -I$(top_builddir)/src/queue -I$(top_builddir)/src/logs \
#	-I$(top_builddir)/src/core $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libhistory.la
#
#noinst_HEADERS = history.h historylistview.h historytab.h mergelistview.h \
#		mergetab.h packageemergetime.h
#
#libhistory_la_SOURCES = history.cpp historylistview.cpp historytab.cpp \
#			historybase.ui mergelistview.cpp mergebase.ui mergetab.cpp
