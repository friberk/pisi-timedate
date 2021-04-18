/*
 * Copyright (C) 2013  Eugene Wissner <belka.ew@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <dbus/dbus.h>

#define BUS_NAME "org.freedesktop.timedate1"
#define BUS_PATH "/org/freedesktop/timedate1"
#define BUS_INTERFACE "org.freedesktop.timedate1"

#define INTROSPECTION_XML DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE \
	"<node>\n" \
	" <interface name=\"org.freedesktop.timedate1\">\n"             \
	"  <property name=\"Timezone\" type=\"s\" access=\"read\"/>\n"  \
	"  <property name=\"LocalRTC\" type=\"b\" access=\"read\"/>\n"  \
	"  <property name=\"NTP\" type=\"b\" access=\"read\"/>\n"       \
	"  <method name=\"SetTime\">\n"                                 \
	"   <arg name=\"usec_utc\" type=\"x\" direction=\"in\"/>\n"     \
	"   <arg name=\"relative\" type=\"b\" direction=\"in\"/>\n"     \
	"   <arg name=\"user_interaction\" type=\"b\" direction=\"in\"/>\n" \
	"  </method>\n"                                                 \
	"  <method name=\"SetTimezone\">\n"                             \
	"   <arg name=\"timezone\" type=\"s\" direction=\"in\"/>\n"     \
	"   <arg name=\"user_interaction\" type=\"b\" direction=\"in\"/>\n" \
	"  </method>\n"                                                 \
	"  <method name=\"SetLocalRTC\">\n"                             \
	"   <arg name=\"local_rtc\" type=\"b\" direction=\"in\"/>\n"    \
	"   <arg name=\"fix_system\" type=\"b\" direction=\"in\"/>\n"   \
	"   <arg name=\"user_interaction\" type=\"b\" direction=\"in\"/>\n" \
	"  </method>\n"                                                 \
	"  <method name=\"SetNTP\">\n"                                  \
	"   <arg name=\"use_ntp\" type=\"b\" direction=\"in\"/>\n"      \
	"   <arg name=\"user_interaction\" type=\"b\" direction=\"in\"/>\n" \
	"  </method>\n"                                                 \
	" </interface>\n"												\
	"</node>\n"

#define DEFAULT_EXIT_SEC 300
#define USEC_PER_SEC  1000000ULL

// Returns the system time zone
gchar *pisi_get_timezone ();

// Sets the system time zone to the one passed by the argument
// Returns true on success, false otherwise
gboolean pisi_set_timezone (gchar *);

// Changes the date/time
// Takes the amount of seconds since UNIX epoche and
// Returns true on success, false otherwise
gboolean pisi_set_time (gint64);
//gboolean pisi_set_time (gint64, gboolean);

// Returns if the hardware clock is set to local time or not
gboolean pisi_get_is_localtime ();

// Returns if NTP is enabled
gboolean pisi_get_ntp ();

// Sets NTP
gboolean pisi_set_ntp (gboolean);
