/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright(C) 2004-2006 William Jon McCann <mccann@jhu.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Authors: William Jon McCann <mccann@jhu.edu>
 *
 */
//#include "config.h"
#include <stdlib.h>
#include <locale.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <glib-unix.h>
//#include "gs-bus.h"

#define GS_SERVICE "org.freedesktop.ScreenSaver"
#define GS_PATH "/org/freedesktop/ScreenSaver"
#define GS_INTERFACE "org.freedesktop.ScreenSaver"

//static gboolean do_lock = FALSE;
//static gboolean do_activate = FALSE;
//static gboolean do_deactivate = FALSE;
//static gboolean do_version = FALSE;
//static gboolean do_poke = FALSE;
//static gboolean do_inhibit = FALSE;
//static gboolean do_uninhibit = FALSE;
//static gboolean do_query = FALSE;
//static gboolean do_time = FALSE;
static char *inhibit_reason = NULL;
static char *inhibit_application = NULL;
static gint32 uninhibit_cookie = 0;
//static int exit_status = EXIT_SUCCESS;
//static gchar **command_argv = NULL;

//static GMainLoop *loop = NULL;
static GDBusMessage *

screensaver_send_message_inhibit(GDBusConnection *connection, const char *application, const char *reason)
{
	GDBusMessage *message, *reply;
	GError *error;
	g_return_val_if_fail(connection != NULL, NULL);
	g_return_val_if_fail(application != NULL, NULL);
	g_return_val_if_fail(reason != NULL, NULL);

	message = g_dbus_message_new_method_call(GS_SERVICE, GS_PATH, GS_INTERFACE, "Inhibit");
	if (message == NULL)
	{
		g_warning("Couldn't allocate the dbus message");
		return NULL;
	}
	g_dbus_message_set_body(message, g_variant_new("(ss)", application, reason));
	error = NULL;
	reply = g_dbus_connection_send_message_with_reply_sync(connection, message, G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &error);
	if (error != NULL)
	{
		g_warning("unable to send message: %s", error->message);
		g_clear_error(&error);
	}
		
	g_dbus_connection_flush_sync(connection, NULL, &error);
	if (error != NULL)
	{
		g_warning("unable to flush message queue: %s", error->message);
		g_clear_error(&error);
	}
	g_object_unref(message);
	return reply;
}

static void screensaver_send_message_uninhibit(GDBusConnection *connection, gint32 cookie)
{
	GDBusMessage *message;
	GError *error;	
	g_return_if_fail(connection != NULL);	
	message = g_dbus_message_new_method_call(GS_SERVICE, GS_PATH, GS_INTERFACE, "UnInhibit");
	if (message == NULL) {
		g_warning("Couldn't allocate the dbus message");
		return;
	}	
	g_dbus_message_set_body(message, g_variant_new("(u)", cookie));	
	error = NULL;
	g_dbus_connection_send_message(connection,
		message,
		G_DBUS_SEND_MESSAGE_FLAGS_NONE,
		NULL,
		&error);
	if (error != NULL)
	{
		g_warning("unable to send message: %s", error->message);
		g_clear_error(&error);
	}	
	g_dbus_connection_flush_sync(connection, NULL, &error);
	if (error != NULL)
	{
		g_warning("unable to flush message queue: %s", error->message);
		g_clear_error(&error);
	}	
	g_object_unref(message);
}



static gboolean parse_reply(GDBusMessage *reply, const gchar *format_string, ...)
{
	va_list   ap;
	GVariant *body;
	GError   *error = NULL;

	if (reply == NULL)
	{
		g_message("Did not receive a reply from the locker.");
		return FALSE;
	}

	if (g_dbus_message_to_gerror(reply, &error))
	{
		g_message("Received error message from the locker: %s", error->message);
		g_error_free(error);
		g_object_unref(reply);
		return FALSE;
	}

	body = g_dbus_message_get_body(reply);
	if (format_string == NULL) {
		if (body != NULL)
		{
			g_warning("Expected empty message");
		}
		g_object_unref(reply);
		return TRUE;
	}
	else if (body == NULL) {
		g_warning("Received empty message");
		g_object_unref(reply);
		return FALSE;
	}

	if (!g_variant_check_format_string(body, format_string, TRUE))
	{
		g_warning("Received incompatible reply");
		g_object_unref(reply);
		return FALSE;
	}

	va_start(ap, format_string);
	g_variant_get_va(body, format_string, NULL, &ap);
	va_end(ap);

	g_object_unref(reply);

	return TRUE;
}



int nonexistent()
{
	GDBusConnection *connection;
	GOptionContext *context;
	gboolean retval;
	GError *error;
	GDBusMessage *reply;

	connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
	if (connection == NULL) {
		g_message ("Failed to get session bus: %s", error->message);
		g_error_free (error);
		return EXIT_FAILURE;
	}
	reply = screensaver_send_message_inhibit(connection, inhibit_application ? inhibit_application : "Unknown", inhibit_reason ? inhibit_reason : "Unknown");
//separator
	parse_reply(reply, "(u)", &uninhibit_cookie);
	screensaver_send_message_uninhibit(connection, uninhibit_cookie);
	g_print(_("Send uninhibit to the screensaver with cookie %d\n"), uninhibit_cookie);
}

