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

#include <gio/gio.h>

#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "pi-glow.h"

/* Gamma-steps from the SN3218 datasheet, e.g.
 * https://github.com/pimoroni/piglow/blob/master/sn3218-datasheet.pdf */
G_GNUC_UNUSED static guint8 GAMMA[32] = {
    0x00, 0x01, 0x02, 0x04, 0x06, 0x0a, 0x0d, 0x12,
    0x16, 0x1c, 0x21, 0x27, 0x2e, 0x35, 0x3d, 0x45,
    0x4e, 0x56, 0x60, 0x6a, 0x74, 0x7e, 0x8a, 0x95,
    0xa1, 0xad, 0xba, 0xc7, 0xd4, 0xe2, 0xf0, 0xff
};

struct _PiGlowPrivate {
	int	fd;
	guint8	enable_reg[3];
	guint	update_id;
};

G_DEFINE_TYPE_WITH_PRIVATE (PiGlow, pi_glow, G_TYPE_OBJECT);

static void pi_glow_class_init (PiGlowClass *klass);
static void pi_glow_init       (PiGlow *self);
static void pi_glow_finalize   (GObject *obj);

static void
pi_glow_class_init (PiGlowClass *klass)
{
        GObjectClass *gobject_class;

        gobject_class = G_OBJECT_CLASS(klass);

        gobject_class->finalize     = pi_glow_finalize;
}

static void
pi_glow_init (PiGlow *self)
{
        self->priv = pi_glow_get_instance_private (self);
}

static void
pi_glow_finalize (GObject *obj)
{
	PiGlow *self;

        self = PI_GLOW(obj);

	if (self->priv->update_id != 0)
		g_source_remove (self->priv->update_id);
	
	if (self->priv->fd != -1) {
		pi_glow_reset (self, NULL);
		close (self->priv->fd);
	}

        G_OBJECT_CLASS(pi_glow_parent_class)->finalize (obj);
}


static gboolean
write_pi_glow (PiGlow *self, guint8 reg, guint8 val, GError **err)
{
	guint8 data[2];

	data[0] = reg;
	data[1] = val;

	if (write (self->priv->fd, data, sizeof(data)) != sizeof(data)) {
		g_set_error (err, G_IO_ERROR, G_IO_ERROR_FAILED,
			     "Failed to write to device");
		return FALSE;
	}

	return TRUE;
}


PiGlow*
pi_glow_new (const char *device, GError **err)
{
        PiGlow *self;

	self = PI_GLOW(g_object_new(PI_TYPE_GLOW, NULL));

	self->priv->fd = open (device, O_RDWR);
	if (self->priv->fd == -1) {
		g_set_error (err, G_IO_ERROR, g_io_error_from_errno(errno),
			     "Failed to initialize PiGlow: %s",
			     strerror (errno));
		goto errexit;
	}

	if (ioctl (self->priv->fd, I2C_SLAVE, 0x54) < 0) {
		g_set_error (err, G_IO_ERROR, g_io_error_from_errno(errno),
			     "Failed to initialize I2C: %s",
			     strerror (errno));
		goto errexit;
	}

	if (!pi_glow_reset (self, err))
		goto errexit;
	
	return self;

errexit:
	g_clear_object (&self);
	return NULL;
	
}

gboolean
pi_glow_reset (PiGlow *self, GError **err)
{
	/* write to reset register */
	if (!write_pi_glow (self, 0x17, 0x01, err))
		return FALSE;

	/* enable again */
	if (!write_pi_glow (self, 0x00, 0x01, err))
		return FALSE;

	self->priv->enable_reg[0] = 0;
	self->priv->enable_reg[1] = 0;
	self->priv->enable_reg[2] = 0;
	
	return TRUE;
}

static guint8
LED_REG[3][6] = {
	/* bottom*/
	/* red, orange, yellow, green, blue, white */
	{ 0x01, 0x02, 0x03, 0x04, 0x0f, 0x0d },
	/* top*/
	/* red, orange, yellow, green, blue, white */
	{ 0x07, 0x08, 0x09, 0x06, 0x05, 0x0a },
	/* right */
	/* red, orange, yellow, green, blue, white */
	{ 0x12, 0x11, 0x10, 0x0e, 0x0c, 0x0b },
};


gboolean
pi_glow_set_led (PiGlow *self, guint8 arm, guint8 led,
		 guint8 intensity, GError **err)
{
	guint8 reg, enable_reg;
	
	g_return_val_if_fail (PI_IS_GLOW(self), FALSE);
	g_return_val_if_fail (arm < 3, FALSE);
	g_return_val_if_fail (led < 6, FALSE);

	/* register for a certain LED */
	reg = LED_REG[arm][5-led];

	if (!write_pi_glow (self, reg, intensity, err))
		return FALSE;
	
	/* if value was set to 0, disable the LED, otherwise enable it */
	/* Bit-wise magic to set the enable flags */

	enable_reg = (reg-1)/6;
	g_return_val_if_fail (enable_reg < 3, FALSE);

	if (intensity != 0)
		self->priv->enable_reg[enable_reg] |= 1 << ((reg-1) % 6);
	else
		self->priv->enable_reg[enable_reg] &= ~(1<<(reg-1) % 6);

	return TRUE;
}

gboolean
pi_glow_update (PiGlow *self, GError **err)
{
	g_return_val_if_fail (PI_IS_GLOW(self), FALSE);

	if (!write_pi_glow (self, 0x13, self->priv->enable_reg[0], err))
		return FALSE;
	if (!write_pi_glow (self, 0x14, self->priv->enable_reg[1], err))
		return FALSE;
	if (!write_pi_glow (self, 0x15, self->priv->enable_reg[2], err))
		return FALSE;
	if (!write_pi_glow (self, 0x16, 0x01, err))
		return FALSE;
	
	return TRUE;
}

typedef struct {
	PiGlow		*self;
	PiGlowFunc	 func;
	gpointer	 user_data;
	guint		 count;
} UpdateData;

static void
update_destroy (UpdateData *updata)
{
	updata->self->priv->update_id = 0;
	g_free (updata);
}

static gboolean
on_update (UpdateData *updata)
{
	GError		*err;
	gboolean	 rv;
	
	rv = updata->func (updata->self, updata->count++, updata->user_data);	

	err = NULL;
	if (!pi_glow_update (updata->self, &err)) {
		g_warning ("%s", err ? err->message : "Update failed");
		g_clear_error (&err);
	}

	return rv ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}


gboolean
pi_glow_animate (PiGlow *self, guint interval,
		 PiGlowFunc func, gpointer user_data)
{
	UpdateData	*updata;
	
	g_return_val_if_fail (PI_IS_GLOW(self), FALSE);
	g_return_val_if_fail (func, FALSE);
	g_return_val_if_fail (self->priv->update_id == 0, FALSE);

	updata		  = g_new0 (UpdateData, 1);
	
	updata->self	  = self;
	updata->func	  = func;
	updata->user_data = user_data;
	updata->count	  = 0;
	
	self->priv->update_id =
		g_timeout_add_full (G_PRIORITY_DEFAULT,
				    interval,
				    (GSourceFunc)on_update,
				    updata,
				    (GDestroyNotify)update_destroy);
	return TRUE;
}
