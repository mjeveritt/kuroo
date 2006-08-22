ADD_SUBDIRECTORY( config ) 
ADD_SUBDIRECTORY( intro ) 
ADD_SUBDIRECTORY( core ) 
ADD_SUBDIRECTORY( history ) 
ADD_SUBDIRECTORY( logs ) 
ADD_SUBDIRECTORY( portage ) 
ADD_SUBDIRECTORY( queue ) 

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/config ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/history ${CMAKE_SOURCE_DIR}/src/intro ${CMAKE_SOURCE_DIR}/src/logs ${CMAKE_SOURCE_DIR}/src/portage ${CMAKE_SOURCE_DIR}/src/queue ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(kuroo_SRCS
kuroo.cpp
main.cpp
kurooview.cpp
mainwindow.cpp
statusbar.cpp
kurooinit.cpp
systemtray.cpp
message.cpp
)

KDE3_AUTOMOC(${kuroo_SRCS})

SET( kuroo_UI
messagebase.ui
kurooviewbase.ui
)

KDE3_ADD_UI_FILES(kuroo_SRCS ${kuroo_UI} )

SET( kuroo_DCOP_SKEL_SRCS
kurooiface.h
)

KDE3_ADD_DCOP_SKELS(kuroo_SRCS ${kuroo_DCOP_SKEL_SRCS})

KDE3_ADD_EXECUTABLE(kuroo ${kuroo_SRCS})

TARGET_LINK_LIBRARIES(kuroo  ${QT_AND_KDECORE_LIBS} config portage history queue logs intro core kdeprint sqlite)

INSTALL_TARGETS(/bin kuroo )


########### install files ###############

INSTALL_FILES( /share/autostart FILES  )
INSTALL_FILES( /share/apps/kuroo FILES  kurooui.rc )
INSTALL_FILES( /share/applnk/System FILES  kuroo.desktop )



#original Makefile.am contents follow:

### Makefile.am for kuroo
#
## this is the program that gets installed.  it's name is used for all
## of the other Makefile.am variables
#bin_PROGRAMS = kuroo
#
## set the include path for X, qt, KDE and subdirs
#INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/config \
#	-I$(top_srcdir)/src/core -I$(top_srcdir)/src/history -I$(top_srcdir)/src/intro \
#	-I$(top_srcdir)/src/logs -I$(top_srcdir)/src/portage -I$(top_srcdir)/src/queue \
#	-I$(top_builddir)/src/ -I$(top_builddir)/src/queue -I$(top_builddir)/src/intro \
#	-I$(top_builddir)/src/history -I$(top_builddir)/src/core -I$(top_builddir)/src/config \
#	-I$(top_builddir)/src/portage -I$(top_builddir)/src/logs $(all_includes)
#
#SUBDIRS = config intro core history logs portage queue
#
## embedded sqlite as library
#SQLITE_SUBDIR = sqlite
#LIB_SQLITE_LOCAL = $(top_builddir)/sqlite/libsqlite.la
#sqlite_includes = -I$(top_srcdir)/sqlite
#
## makes pot translation files
#messages: rc.cpp
#	$(EXTRACTRC) `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` > rc.cpp
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#		$(XGETTEXT) $$LIST -o $(podir)/kuroo.pot; \
#	fi
#
## this is where the kdelnk file will go 
#kdelnkdir = $(kde_appsdir)/System
#kdelnk_DATA = kuroo.desktop
#
## this is where the XML-GUI resource file goes
#rcdir = $(kde_datadir)/kuroo
#rc_DATA = kurooui.rc
#
## this is to set set autostart of kuroo_watcher
#autostartdir = $(datadir)/autostart
#
## headers for kuroo
#noinst_HEADERS = kurooview.h kurooiface.h singleton.h mainwindow.h statusbar.h \
#	systemtray.h message.h common.h
#
## source for kuroo
#kuroo_SOURCES = kurooiface.skel kuroo.cpp main.cpp kurooview.cpp mainwindow.cpp \
#	statusbar.cpp kurooinit.cpp systemtray.cpp message.cpp messagebase.ui \
#	kurooviewbase.ui
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#kuroo_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#
## kuroo links
#kuroo_LDADD = $(top_builddir)/src/history/libhistory.la \
#	$(top_builddir)/src/config/libconfig.la $(top_builddir)/src/queue/libqueue.la \
#	$(top_builddir)/src/portage/libportage.la $(top_builddir)/src/logs/liblogs.la \
#	$(top_builddir)/src/intro/libintro.la $(top_builddir)/src/core/libcore.la $(LIB_KDEPRINT) $(LIB_KFILE) \
#	$(LIB_KDECORE) $(LIB_SQLITE) $(LIB_SQLITE_LOCAL)
#
#
