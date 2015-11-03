/*
** Copyright (C) 2015 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 3, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation,
** Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**
*/

#include <glib.h>
#include <glib-unix.h>

#include <stdlib.h>
#include "pi-glow.h"

static gboolean xmas (PiGlow *pig, guint count, gpointer ignored);
static gboolean rotate  (PiGlow *pig, guint count, gpointer ignored);


static PiGlowFunc
get_effect_func (const char *name)
{
	guint u;

	struct {
		const char	*name;
		PiGlowFunc	 func;
	} effects[] = {
		{"xmas", xmas},
		{"rotate",  rotate},
	};

	for (u = 0; u != G_N_ELEMENTS(effects); ++u)
		if (g_strcmp0 (name, effects[u].name) == 0)
			return effects[u].func;

	g_printerr ("Unknown effect '%s'; valid effects are:\n", name);
	for (u = 0; u != G_N_ELEMENTS(effects); ++u)
		g_printerr ("\t%s\n", effects[u].name);
	
	return NULL;
}


int
main (int argc, char *argv[])
{
	GError		*err;
	PiGlow		*pig;
	GMainLoop	*loop;
	PiGlowFunc	 func;
	gint		 interval;
	
	if (argc < 3 || argc > 4) {
		g_printerr ("usage: piglow <device> <effect> "
			    "[<interval in millisec>]\n");
		return 1;
	}

	func = get_effect_func (argv[2]);
	if (!func)
		return 1;

	interval = (argc == 4) ? atoi (argv[3]) : 200;
	if (interval <= 0) {
		g_printerr ("Invalid interval\n");
		return 1;
	}
		
	err = NULL;
	pig = pi_glow_new (argv[1], &err);
	if (!pig)
		goto errexit;

	if (!pi_glow_animate (pig, interval, func, NULL))
		goto errexit;
	
	loop = g_main_loop_new (NULL, TRUE);	
	g_unix_signal_add (SIGINT, (GSourceFunc)g_main_loop_quit, loop);
	g_print ("Press Ctrl-C to quit\n");
	g_main_loop_run (loop);
	
	g_clear_object (&pig);
	g_main_loop_unref (loop);
	
	return 0;
errexit:
	g_printerr ("Error: %s\n", err ? err->message :
		    "something went wrong\n");
	g_clear_error (&err);
	g_clear_object (&pig);

	return 1;
}



static gboolean
xmas (PiGlow *pig, guint count, gpointer ignored)
{
	GError	*err;
	guint	 intensity, u;

	err	  = NULL;
	intensity = 256 / ((count % 6) + 1);
	
	if (!pi_glow_reset (pig, &err))
		goto errexit;
	
	for (u = 0; u < 3; ++u) {
		if (!pi_glow_set_led (pig, u, ((count + 3) % 6),
				      intensity, &err))
			goto errexit;	
		if (!pi_glow_set_led (pig, u, (count % 6),
				      intensity, &err))
			goto errexit;
	}
		
	return TRUE;

errexit:
	g_warning ("%s", err ? err->message : "something went wrong");
	g_clear_error (&err);

	return FALSE;
}


static gboolean
rotate (PiGlow *pig, guint count, gpointer ignored)
{
	GError	*err;
	guint	 u;

	err	 = NULL;
	if (!pi_glow_reset (pig, &err))
		goto errexit;
			
	for (u = 0; u < 6; ++u)
		if (!pi_glow_set_led (pig, (count % 3), u, 10/(7-u), &err))
			goto errexit;
	
	return TRUE;

errexit:
	g_warning ("%s", err ? err->message : "something went wrong");
	g_clear_error (&err);

	return FALSE;
}

