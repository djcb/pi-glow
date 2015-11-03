/*
** Copyright (C) 2015 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public License
**  as published by the Free Software Foundation; either version 2.1
**  of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free
**  Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
**  02110-1301, USA.
*/


#ifndef __PI_GLOW_H__
#define __PI_GLOW_H__

#include <glib-object.h>

/**
 * SECTION:pi-glow
 * @short_description: a GObject to control the PiGlow
 * @title: PiGlow
 * @include: pi-glow.h
 *
 * A simple #GObject to control the PiGLow - the LED device for the Raspberry
 * Pi.
 */

G_BEGIN_DECLS

#define PI_TYPE_GLOW             (pi_glow_get_type())
#define PI_GLOW(self)            (G_TYPE_CHECK_INSTANCE_CAST((self), \
                                     PI_TYPE_GLOW,PiGlow))
#define PI_GLOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),   \
				     PI_TYPE_GLOW,PiGlowClass))
#define PI_IS_GLOW(self)         (G_TYPE_CHECK_INSTANCE_TYPE((self), \
                                     PI_TYPE_GLOW))
#define PI_IS_GLOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),   \
                                     PI_TYPE_GLOW))
#define PI_GLOW_GET_CLASS(self)  (G_TYPE_INSTANCE_GET_CLASS((self),  \
                                     PI_TYPE_GLOW,PiGlowClass))

typedef struct _PiGlow        PiGlow;
typedef struct _PiGlowClass   PiGlowClass;
typedef struct _PiGlowPrivate PiGlowPrivate;


/**
 * PiGlow:
 *
 * The #PiGlow structure contains only private data and should be
 * accessed using its public API
 *
 */
struct _PiGlow {
        /*< private >*/
	GObject		 parent;
	PiGlowPrivate	*priv;
};

/**
 * PiGlowClass:
 * 
 * Class structure for #PiGlow. The #PiGlowClass structure contains
 * only private data and should be accessed using its public API.
 */
struct _PiGlowClass {
	/*< private >*/
	GObjectClass	parent_class;
};

/**
 * pi_glow_get_type:
 * 
 * Get the #GType for #PiGlow
 * 
 * Returns: the #GType
 */
GType pi_glow_get_type    (void) G_GNUC_CONST;

/**
 * pi_glow_new:
 * @err: (allow-none): receives error information
 *
 * Example constructor function to get a new #PiGlow instance
 *
 * Returns: (transfer full): a new #PiGlow instance or %NULL
 * in case of error. Free with g_object_unref().
 */
PiGlow* pi_glow_new (const char *device, GError **err)
	G_GNUC_WARN_UNUSED_RESULT;

/**
 * pi_glow_reset:
 * @self: a #PiGlow instance
 * @err: (allow-none): receives error information
 *
 * Reset the LEDs to their default values.
 * 
 * Return value: %TRUE if resetting the LEDs worked; %FALSE otherwise.
 */
gboolean pi_glow_reset (PiGlow *self, GError **err);

/**
 * pi_glow_set_led:
 * @self: a #PiGlow instance
 * @arm: the "arm" [0..2] to change. The arms are respectively bottom, top and
 * right.
 * @led: the LED to set [0..5], counted from 0 in the center to 5 (farthest
 * away). The colors for each of the LEDs are resp. white, blue, green, yellow,
 * orange and red.
 * @intesity: the intensity, from 0 for 'off' until 255 for maximum.
 * @err: (allow-none): receives error information
 *
 * Set some LED. After one or more calls to pi_glow_set_led(), use
 * pi_glow_update() to effectuate the changes.
 *
 * Return value: %TRUE if the setting the LED worked; %FALSE otherwise.
 */
gboolean pi_glow_set_led (PiGlow *self, guint8 arm,
			  guint8 led, guint8 intensity,
			  GError **err);

/**
 * pi_glow_update:
 * @self: a #PiGlow instance
 * @err: (allow-none): receives error information
 *
 * Effectuate the changes in the LEDs; see pi_glow_set_led().
 * 
 * Return value: %TRUE if updating the LEDs worked; %FALSE otherwise.
 */
gboolean pi_glow_update (PiGlow *self, GError **err);



/**
 * PiGlowFunc:
 * @self: a #PiGFlow instance
 * @count: the sequence number ([0...])
 * @user_data: the user-data passed to @pi_glow_animate
 *
 * Prototype for a function to do one 'frame' of an animation.
 *
 * Return value: %TRUE if this animation should continue, %FALSE if we need to
 * stop.
 */
typedef gboolean (*PiGlowFunc) (PiGlow *self, guint count, gpointer user_data);


/**
 * pi_glow_animate:
 * @self: a #PiGlow instance
 * @interval: update-interval, in millisecs
 * @func: #PiGlowFunc function to call each interval
 * @user_data: user-pointer passed to @func
 *
 * Call a function each @interval millisecs. After the function returns, calls
 * pi_glow_update(). Assumes a running #GMainLoop.
 *
 * Return value: %TRUE if starting the run worked; %FALSE otherwise.
 * */
gboolean pi_glow_animate (PiGlow *self, guint interval,
			  PiGlowFunc func, gpointer user_data);


G_END_DECLS

#endif /* __PI_GLOW_H__ */

