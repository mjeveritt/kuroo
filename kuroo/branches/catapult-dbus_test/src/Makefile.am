# set the include path for X, qt and KDE
INCLUDES = $(all_includes) -Idbus-qt3-backport $(DBUS_CFLAGS) -DDBUS_API_SUBJECT_TO_CHANGE

# these are the headers for your project
noinst_HEADERS = dbus_test1.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/dbus_test1.pot

KDE_ICON = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = dbus_test1

# the application source, library search path, and link libraries
dbus_test1_SOURCES = main.cpp dbus_test1.cpp
dbus_test1_LDFLAGS = $(KDE_RPATH) $(all_libraries)
dbus_test1_LDADD = dbus-qt3-backport/libdbus-1-qt3.la $(LIB_KDEUI) $(DBUS_LIBS)

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = dbus_test1.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/dbus_test1
shellrc_DATA = dbus_test1ui.rc

SUBDIRS = dbus-qt3-backport

KDE_CXXFLAGS = $(USE_EXCEPTIONS)