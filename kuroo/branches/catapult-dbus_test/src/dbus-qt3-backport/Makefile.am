INCLUDES=-I$(srcdir) $(all_includes) $(DBUS_CFLAGS) \
	     -DDBUS_COMPILATION -DDBUS_API_SUBJECT_TO_CHANGE

dbusincludedir=$(includedir)/dbus-1.0/qt3/dbus

noinst_LTLIBRARIES=libdbus-1-qt3.la

libdbus_1_qt3_la_SOURCES =                   \
	dbus/qdbusconnection.h   \
	dbus/qdbuserror.h   \
	dbus/qdbusmacros.h   \
	dbus/qdbusmessage.h   \
	dbus/qdbusobject.h   \
	dbus/qdbusproxy.h   \
	dbus/qdbusserver.h   \
	dbus/qdbusvariant.h   \
	qdbusconnection_p.h    \
	qdbusconnection.cpp    \
	qdbuserror.cpp         \
	qdbusintegrator.cpp    \
	qdbusmarshall.cpp      \
	qdbusmessage.cpp       \
	qdbusserver.cpp        \
	qdbusproxy.cpp

$(srcdir)/qdbusserver.cpp: qdbusserver.moc
$(srcdir)/qdbusconnection.cpp: qdbusconnection.moc
$(srcdir)/qdbusproxy.cpp: qdbusproxy.moc

CLEANFILES = qdbusserver.moc qdbusconnection.moc qdbusproxy.moc qdbusconnection_p.moc.cpp

DISTCLEANFILES = dbus-1-qt3.pc

# files that are an exception
qdbusserver.moc: dbus/qdbusserver.h
	$(MOC) -o qdbusserver.moc $(srcdir)/dbus/qdbusserver.h

qdbusconnection.moc: qdbusconnection_p.h
	$(MOC) -o qdbusconnection.moc $(srcdir)/qdbusconnection_p.h

qdbusproxy.moc: dbus/qdbusproxy.h
	$(MOC) -o qdbusproxy.moc $(srcdir)/dbus/qdbusproxy.h

