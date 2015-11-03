## Copyright (C) 2015 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
##
##  This library is free software; you can redistribute it and/or
##  modify it under the terms of the GNU Lesser General Public License
##  as published by the Free Software Foundation; either version 2.1
##  of the License, or (at your option) any later version.
##
##  This library is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##  Lesser General Public License for more details.
##
##  You should have received a copy of the GNU Lesser General Public
##  License along with this library; if not, write to the Free
##  Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
##  02110-1301, USA.

CFLAGS=-fPIC -Wall -Wextra -Wno-unused-parameter -O0 -g `pkg-config --cflags glib-2.0 gio-2.0`
LIBS= `pkg-config --libs glib-2.0 gio-2.0`

piglow: piglow-main.c pi-glow.c pi-glow.h
	$(CC) -o piglow piglow-main.c pi-glow.c $(CFLAGS) $(LIBS)

clean:
	-${RM} libpiglow.so piglow
