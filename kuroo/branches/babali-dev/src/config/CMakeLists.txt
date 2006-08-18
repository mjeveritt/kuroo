INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/config ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/history ${CMAKE_SOURCE_DIR}/src/intro ${CMAKE_SOURCE_DIR}/src/logs ${CMAKE_SOURCE_DIR}/src/portage ${CMAKE_SOURCE_DIR}/src/queue ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(config_STAT_SRCS
configdialog.cpp
)

KDE3_AUTOMOC(${config_STAT_SRCS})

SET( config_UI
options1.ui
options2.ui
options7.ui
)

KDE3_ADD_UI_FILES(config_STAT_SRCS ${config_UI} )

SET( config_KCFG_SRCS
settings.kcfgc
)

KDE3_ADD_KCFG_FILES(config_STAT_SRCS ${config_KCFG_SRCS})

ADD_LIBRARY(config STATIC ${config_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /share/config.kcfg FILES  kurooconfig.kcfg )



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
#	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
#	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
#	-I$(top_builddir)/src/core $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libconfig.la
#
#libconfig_la_SOURCES = settings.kcfgc configdialog.cpp options1.ui options2.ui \
#	options7.ui
#
#noinst_HEADERS = configdialog.h
#
#kde_kcfg_DATA = kurooconfig.kcfg
