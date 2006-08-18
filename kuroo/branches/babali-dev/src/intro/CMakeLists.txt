INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/config ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/intro ${CMAKE_SOURCE_DIR}/src/logs ${CMAKE_SOURCE_DIR}/src/portage ${CMAKE_SOURCE_DIR}/src/queue ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(intro_STAT_SRCS
introdlg.cpp
)

KDE3_AUTOMOC(${intro_STAT_SRCS})

SET( intro_UI
intro.ui
)

KDE3_ADD_UI_FILES(intro_STAT_SRCS ${intro_UI} )

ADD_LIBRARY(intro STATIC ${intro_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
#	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/intro -I$(top_srcdir)/src/logs \
#	-I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue -I$(top_builddir)/src/core \
#	-I$(top_builddir)/src/config $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libintro.la
#
#libintro_la_SOURCES = intro.ui introdlg.cpp
#
#noinst_HEADERS = introdlg.h
#
