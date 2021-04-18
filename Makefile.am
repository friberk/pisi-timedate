dbus_servicesdir = $(datadir)/dbus-1/system-services
dbus_confdir = $(sysconfdir)/dbus-1/system.d
introspectiondir = $(datadir)/dbus-1/interfaces

libexec_PROGRAMS = pisi-timedate

INCLUDES = $(GDBUS_CFLAGS)

pisi_timedate_SOURCES = pisi-timedate.c pisi-timedate.h
pisi_timedate_LDADD = $(GDBUS_LIBS)

dbus_services_DATA = org.freedesktop.timedate1.service
$(dbus_services_DATA): org.freedesktop.timedate1.service.in
	$(AM_V_GEN)sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@
dbus_conf_DATA = org.freedesktop.timedate1.conf

introspection_DATA = org.freedesktop.timedate1.xml

EXTRA_DIST = \
	$(dbus_services_DATA) \
	$(dbus_conf_DATA) \
	$(introspection_DATA)

CLEANFILES = \
	org.freedesktop.timedate1.service
