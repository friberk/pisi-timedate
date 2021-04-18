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

#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "pisi-timedate.h"

static void pisi_method_call (GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, gpointer user_data) {
	gchar *timezone, *response;
	gboolean user_interaction, relative, is_localtime, use_ntp;
	gint64 usec_utc;

	// Set time zone
	if (g_strcmp0 (method_name, "SetTimezone") == 0) {
		g_variant_get (parameters, "(&sb)", &timezone, &user_interaction);

		if (pisi_set_timezone (timezone)) g_dbus_method_invocation_return_value (invocation,
				NULL);
		else g_dbus_method_invocation_return_error (invocation,
				G_IO_ERROR,
				G_IO_ERROR_FAILED,
				"Write operation failed");

		g_free (timezone);
    } else if (g_strcmp0 (method_name, "SetTime") == 0) {
		g_variant_get (parameters, "(xbb)", &usec_utc, &relative, &user_interaction);

		// Set time
		//if (!pisi_set_time (usec_utc, pisi_get_is_localtime ())) {
		if (pisi_set_time (usec_utc)) g_dbus_method_invocation_return_value (invocation,
				NULL);
		else g_dbus_method_invocation_return_error (invocation,
				G_IO_ERROR,
				G_IO_ERROR_FAILED,
				"Failed to set system clock");
    } else if (g_strcmp0 (method_name, "SetNTP") == 0) {
		g_variant_get (parameters, "(bb)", &use_ntp, &user_interaction);

		// Enable NTP
		if (pisi_set_ntp (use_ntp)) g_dbus_method_invocation_return_value (invocation,
				NULL);
		else g_dbus_method_invocation_return_error (invocation,
				G_IO_ERROR,
				G_IO_ERROR_FAILED,
				"Error enabling NTP");
	}
	return ;
}

static GVariant *pisi_get_property (GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *prop_name, GError **pisi_err, gpointer user_data) {
	if (g_strcmp0 ("Timezone", prop_name) == 0) {
		return g_variant_new_string (pisi_get_timezone ());
	} if (g_strcmp0 ("LocalRTC", prop_name) == 0) {
		return g_variant_new_boolean (pisi_get_is_localtime ());
	} if (g_strcmp0 ("NTP", prop_name) == 0) {
		return g_variant_new_boolean (pisi_get_ntp ());
	}
	return NULL;
}

static void on_timedate_acquired (GDBusConnection *connection, const gchar *name, gpointer user_data) {
	guint registration_id;
	GDBusNodeInfo *introspection_data;
	GError *pisi_err;
	const GDBusInterfaceVTable interface_vtable = {
		pisi_method_call,
		pisi_get_property,
		NULL
	};

	pisi_err = NULL;
	introspection_data = g_dbus_node_info_new_for_xml (INTROSPECTION_XML, &pisi_err);
	if (introspection_data == NULL)
		g_error ("Failed to parse D-Bus introspection XML: %s\n", pisi_err->message);

	pisi_err = NULL;
	registration_id = g_dbus_connection_register_object (connection,
			BUS_PATH,
			introspection_data->interfaces[0],
			&interface_vtable,
			NULL,
			NULL,
			&pisi_err);

	if (registration_id <= 0) {
		g_critical ("Failed to register callbacks for the exported object with the D-Bus interface: %s\n", pisi_err->message);
		g_error_free (pisi_err);
	}

	g_dbus_node_info_unref (introspection_data);
}

static void on_timedate_lost (GDBusConnection *connection, const gchar *name, gpointer user_data) {
	g_warning ("Failed to acquire the service %s.\n", name);
	exit (1);
}

gboolean timeout_callback (gpointer loop2quit) {
	g_main_loop_quit ((GMainLoop *)loop2quit);
	return FALSE;
}

int main (int argc, char **argv) {
	guint owner_id;
	GMainLoop *loop;

	g_type_init ();

	owner_id = g_bus_own_name (G_BUS_TYPE_SYSTEM,
			BUS_NAME,
			G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
			G_BUS_NAME_OWNER_FLAGS_REPLACE,
			on_timedate_acquired,
			NULL,
			on_timedate_lost,
			NULL,
			NULL);

	loop = g_main_loop_new (NULL, FALSE);
	g_timeout_add_seconds (DEFAULT_EXIT_SEC, timeout_callback , loop);
	g_main_loop_run (loop);

	g_bus_unown_name (owner_id);

	return EXIT_SUCCESS;
}


gchar *pisi_get_timezone () {
	gchar *zone_copied_from, *zone;

	zone_copied_from = g_file_read_link ("/etc/localtime-copied-from", NULL);
	if (zone_copied_from == NULL) return NULL;

	zone = g_strdup (zone_copied_from + strlen ("/usr/share/zoneinfo/") * sizeof (gchar));
	g_free (zone_copied_from);

	return zone;
}

gboolean pisi_set_timezone (gchar *zone) {
	gchar *zone_file;
	GFile *etc_localtime, *localtime_link, *g_zone_file;

	zone_file = g_strconcat ("/usr/share/zoneinfo/", zone, NULL);
	if (g_file_test (zone_file, G_FILE_TEST_IS_REGULAR)) {
		etc_localtime = g_file_new_for_path ("/etc/localtime");
		localtime_link = g_file_new_for_path ("/etc/localtime-copied-from");
		g_zone_file = g_file_new_for_path (zone_file);

		if (!g_file_copy (g_zone_file, etc_localtime, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL)) {
			return FALSE;
		}

		if (!g_file_delete (localtime_link, NULL, NULL)) {
			return FALSE;
		}
		if (!g_file_make_symbolic_link (localtime_link, zone_file, NULL, NULL)) {
			return FALSE;
		}

		g_free (zone_file);
		g_object_unref (etc_localtime);
		g_object_unref (localtime_link);
		g_object_unref (g_zone_file);
	} else {
		return FALSE;
	}
	return TRUE;
}

gboolean pisi_set_time (gint64 seconds_since_epoch) {
	struct timespec ts;
/*	gint exit_status;
	gboolean spawn_status;
	gchar *cmd;*/

	// Set system clock
	ts.tv_sec = (time_t) (seconds_since_epoch / USEC_PER_SEC);
	ts.tv_nsec = 0;
	if (clock_settime (CLOCK_REALTIME, &ts)) {
		return FALSE;
	}

	// Don't save the system time to the hardware clock. The saving takes much
	// time. Anyway it is saved automatically on shutdown.
	/* Set hardware clock
	if (!g_file_test ("/sbin/hwclock", G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_EXECUTABLE)) {
		return FALSE;
	}
	cmd = g_strdup_printf ("/sbin/hwclock %s --systohc", is_localtime ? "--localtime" : "-u");
	spawn_status = g_spawn_command_line_sync (cmd, NULL, NULL, &exit_status, NULL);
	g_free (cmd);
	if ((WEXITSTATUS (exit_status) != 0) || !spawn_status) {
		return FALSE;
	}*/

	return TRUE;
}

gboolean pisi_get_is_localtime () {
	FILE *fh;
	char time_str[10]; // "localtime" is longer than "UTC" and has 9 letters + \0
	gboolean is_localtime;

	is_localtime = FALSE;

	fh = fopen ("/etc/hardwareclock", "r");
	if (fh == NULL) return FALSE;
	while (fgets (time_str, 10, fh)) {
		if (!g_strcmp0 (time_str, "localtime")) {
			is_localtime = TRUE;
			break;
		}
	}

	fclose (fh);
	return is_localtime;
}

gboolean pisi_get_ntp () {
	if (g_file_test ("/etc/rc.d/rc.ntpd", G_FILE_TEST_IS_EXECUTABLE))
		return TRUE;
	else return FALSE;
}

gboolean pisi_set_ntp (gboolean xntp) {
	mode_t rc_mode;

	rc_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	if (xntp) {
		rc_mode |= S_IXUSR | S_IXGRP | S_IXOTH;
		g_message ("Please don't forget to configure the NTP daemon");

		/* It doesn't matter if fails.
		 * The ntpdate command is considered obsolete, but "orthodox" ntpd -g
		 * will fail if your system clock is off for more than half an hour. */
		g_spawn_command_line_async ("/usr/sbin/ntpdate pool.ntp.org", NULL);
	}

	if (g_file_test ("/etc/rc.d/rc.ntpd", G_FILE_TEST_IS_REGULAR)) {
		if (chmod ("/etc/rc.d/rc.ntpd", rc_mode)) return FALSE;
		else return TRUE;
	} else {
		g_error ("The NTP daemon isn't installed");
		return FALSE;
	}
}
