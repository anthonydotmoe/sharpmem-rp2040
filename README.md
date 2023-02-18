# sharpmem-rp2040
Sharp Memory Display Library for RP2040

I wanted to use the [Adafruit 400x240 Sharp Memory Display]() for my [VGM Player
project](). I couldn't find any existing libraries to interface with this display
so I started my own. I'm a novice with C library conventions but this is my go
at it.

These displays want the least-significant bit first over the SPI bus, but the
RP2040's SPI peripheral doesn't support this. I ended up using a byte reversal
array to send the data in the required format, but I might use one of the PIO
state machines as an SPI peripheral that sends the data LSB first.

Lots of code is borrowed from the [Adafruit\_GFX](https://github.com/adafruit/Adafruit_GFX)
and [Adafruit\_SHARP\_Memory\_Display](https://github.com/adafruit/Adafruit_SHARP_Memory_Display)
libraries.
