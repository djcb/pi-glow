# PiGlow

PiGlow is a `GObject` for manipulating the
[PiGlow](https://shop.pimoroni.com/products/piglow) add-on board for
the The Raspberry Pi. It wraps the low-level communication in a
high-level API. This makes it very convenient to program the PiGlow
from C and C++.

Apart from the functions to get/set the LEDs, there is a function
`pi_glow_animate`, which allows you to define animations, frame by
frame, using a callback-function.

Usage is straightforward; see [piglow-main.c](piglow-main.c) for an
example.
