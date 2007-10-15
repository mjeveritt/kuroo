HEADERS += qdbusconnection.h \
           qdbusmessage.h \
           qdbusmarshall.h \
           qdbuserror.h \
           qdbus.h \
           qdbusconnection_p.h \
           qdbusserver.h \
           qdbusmacros.h \
           qdbusvariant.h

SOURCES += qdbusconnection.cpp \
           qdbusmessage.cpp \
           qdbusmarshall.cpp \
           qdbuserror.cpp \
           qdbusserver.cpp \
           qdbusintegrator.cpp

TEMPLATE = lib
DEPENDPATH += .
CONFIG += create_prl link_pkgconfig
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

PKGCONFIG += dbus-1
system(pkg-config --max-version=0.29 dbus-1):DEFINES+=QDBUS_V2

DEFINES += DBUS_API_SUBJECT_TO_CHANGE QDBUS_MAKEDLL QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
QT = core

QTDIR           = $(QTDIR)
# TODO - make independent of QTDIR
MOC_DIR         = tmp

CONFIG(debug, debug|release) {
    TARGET = QtDBUS_debug
    OBJECTS_DIR = tmp/debug
    system(cat QtDBUS_debug.pc.in | sed -e "s,@QTDIR@,$QTDIR," > QtDBUS_debug.pc) {
        targ_pkgconfig.files *= QtDBUS_debug.pc
    }
} else {
    TARGET = QtDBUS
    OBJECTS_DIR = tmp/release
    system(cat QtDBUS.pc.in | sed -e "s,@QTDIR@,$QTDIR," > QtDBUS.pc) {
        targ_pkgconfig.files *= QtDBUS.pc
    }
}

INSTALL_HEADERS = $$HEADERS
targ_headers.files = $$INSTALL_HEADERS
targ_headers.path = $$[QT_INSTALL_HEADERS]/QtDBUS
targ_pkgconfig.path = $$[QT_INSTALL_LIBS]/pkgconfig
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target targ_headers targ_pkgconfig
