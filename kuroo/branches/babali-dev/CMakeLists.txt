PROJECT(Kuroo C CXX)

SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_COLOR_MAKEFILE ON)

FIND_PACKAGE(KDE3 REQUIRED)

MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir ${TOPSUBDIRS}")

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE3_DEFINITIONS})

LINK_DIRECTORIES(${KDE3_LIB_DIR})

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

ADD_SUBDIRECTORY(doc)
ADD_SUBDIRECTORY(pics)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(sqlite)



########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = $(TOPSUBDIRS)
#
#$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
#	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;
#
#$(top_srcdir)/subdirs:
#	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs
#
#$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
#	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4
#
#MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 
#
#package-messages:
#	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common package-messages
#	$(MAKE) -C po merge
#
#EXTRA_DIST = admin COPYING configure.in.in
#
#dist-hook:
#	cd $(top_distdir) && perl admin/am_edit -padmin
#	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs
#
#include $(top_srcdir)/admin/Doxyfile.am
#
