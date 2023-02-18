#include "sharp.h"

/*
   - Internal functions are preceded with sharp_
   - User facing functions are preceded with sharp_display_

   - Many things taken from the Adafruit_SHARP_Memory_Display Arduino Library
    and the Adafruit_GFX library.
*/
    
// Display SPI commands
#define SHARP_BIT_WRITECMD (0x01)
#define SHARP_BIT_VCOM     (0x02)
#define SHARP_BIT_CLEAR    (0x04)

#define TOGGLE_VCOM(x)                 \
        x = x ? 0x00 : SHARP_BIT_VCOM;

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
    {                       \
        int16_t t = a;      \
        a = b;              \
        b = t;              \
    }                
#endif
#ifndef _swap_uint16_t
#define _swap_uint16_t(a, b) \
    {                        \
        uint16_t t = a;      \
        a = b;               \
        b = t;               \
    }                
#endif

/*Array of reversed bytes.  
This array is used to reverse bytes sent to the sharp memory lcd.  For speed
it is kept in memory.  
Future TODO - test placement into program memory / function call use.  Will cause
a speed hit, but that may not have an impact if comms with the LCD is still the 
longest delay.
*/
unsigned char revByte[256] = {0x0,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,0x10,0x90,
0x50,0xd0,0x30,0xb0,0x70,0xf0,0x8,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,0x18,0x98,
0x58,0xd8,0x38,0xb8,0x78,0xf8,0x4,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,0x14,0x94,
0x54,0xd4,0x34,0xb4,0x74,0xf4,0xc,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,0x1c,0x9c,
0x5c,0xdc,0x3c,0xbc,0x7c,0xfc,0x2,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,0x12,0x92,
0x52,0xd2,0x32,0xb2,0x72,0xf2,0xa,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,0x1a,0x9a,
0x5a,0xda,0x3a,0xba,0x7a,0xfa,0x6,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,0x16,0x96,
0x56,0xd6,0x36,0xb6,0x76,0xf6,0xe,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,0x1e,0x9e,
0x5e,0xde,0x3e,0xbe,0x7e,0xfe,0x1,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,0x11,0x91,
0x51,0xd1,0x31,0xb1,0x71,0xf1,0x9,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,0x19,0x99,
0x59,0xd9,0x39,0xb9,0x79,0xf9,0x5,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,0x15,0x95,
0x55,0xd5,0x35,0xb5,0x75,0xf5,0xd,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,0x1d,0x9d,
0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,0x3,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,0x13,0x93,
0x53,0xd3,0x33,0xb3,0x73,0xf3,0xb,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,0x1b,0x9b,
0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,0x7,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,0x17,0x97,
0x57,0xd7,0x37,0xb7,0x77,0xf7,0xf,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,0x1f,0x9f,
0x5f,0xdf,0x3f,0xbf,0x7f,0xFF};

/// @brief Transmit a byte over SPI with bits reversed
/// @param spi SPI peripheral to use
/// @param data Byte to transmit
/// @return Number of bytes sent
int sharp_spi_transmit(spi_inst_t *spi, uint8_t data) {
    uint8_t rev_data = revByte[data];
    return spi_write_blocking(spi, &rev_data, 1);
}

/// @brief Transmit a byte array over SPI with each byte's bits reversed
/// @param spi SPI peripheral to use
/// @param data Buffer of data to write
/// @param len Length of data
/// @return Number of bytes sent
int sharp_spi_transmit_seq(spi_inst_t *spi, uint8_t *data, size_t len) {
    int ret;
    // Allocate space for copy of data
    uint8_t *rev_data = malloc(len * sizeof(uint8_t));
    // Reverse bits in each byte in the copy
    for(int i = 0; i < len; i++) {
        rev_data[i] = revByte[data[i]];
    }
    // Transfer the array
    ret = spi_write_blocking(spi, rev_data, len);
    // Free the borrowed memory
    free(rev_data);
    return ret;
}

static inline void sharp_clear_display_buffer(struct sharp_display_context *ctx) {
    memset(ctx->buf, 0xff, (ctx->width * ctx->height) / 8);
}

/// @brief Create Sharp memory display library context
/// @param c Configuration structure
struct sharp_display_context *sharp_display_create_context(struct sharp_display_config *c) {
    struct sharp_display_context ctx_data = {
        .spi = c->spi,
        .cs = c->cs,
        .width = c->width,
        .height = c->height,
        .rotation = c->rotation,
        .buf = malloc((c->width * c->height) / 8),
        .vcom = SHARP_BIT_VCOM
    };
    
    if(!ctx_data.buf) {
        return NULL;
    }

    struct sharp_display_context *ctx = malloc(sizeof(struct sharp_display_context));
    if(!ctx) {
        return NULL;
    }
    
    memcpy(ctx, &ctx_data, sizeof(struct sharp_display_context));
    
    // Initialize peripherals
    spi_init(ctx->spi, 2000000);
    spi_set_format(ctx->spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_LSB_FIRST);
    uint8_t throwaway;
    spi_read_blocking(ctx->spi, 0, &throwaway, 1);

    gpio_init(ctx->cs);
    gpio_set_dir(ctx->cs, GPIO_OUT);
    gpio_put(ctx->cs, 0); // Active high!

    sharp_clear_display_buffer(ctx);
    
    return ctx;
}

void sharp_display_free_context(struct sharp_display_context *ctx) {
    free(ctx->buf);
    free(ctx);
    return;
}

/// @brief Draw a single pixel in image buffer
/// @param ctx Display context
/// @param x The x position (0 based)
/// @param y The x position (0 based)
/// @param color The color to set: 0 for Black, 1 for White
void sharp_display_draw_pixel(struct sharp_display_context *ctx, int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= ctx->width) || (y < 0) || (y >= ctx->height))
        return;
    
    switch(ctx->rotation) {
        case 1:
            _swap_int16_t(x, y);
            x = ctx->width - 1 - x;
            break;
        case 2:
            x = ctx->width - 1 - x;
            y = ctx->height - 1 - y;
            break;
        case 3:
            _swap_int16_t(x, y);
            y = ctx->height - 1 - y;
            break;
    }
    
    size_t index = (y * ctx->width + x) / 8;
    
    if(color) {
        ctx->buf[index] |= (1 << (x & 7));
    } else {
        ctx->buf[index] &= ~(1 << (x & 7));
    }
}

/// @brief Gets the value (1 or 0) of the specified pixel from the buffer
/// @param ctx Display context
/// @param x The x position (0 based)
/// @param y The y position (0 based)
/// @return 1 if the pixel is enabled, 0 if disabled
uint8_t sharp_display_get_pixel(struct sharp_display_context *ctx, uint16_t x, uint16_t y) {
    if ((x >= ctx->width) || (y >= ctx->height))
        return 0;
    
    switch (ctx->rotation) {
        case 1:
            _swap_uint16_t(x, y);
            x = ctx->width - 1 - x;
            break;
        case 2:
            x = ctx->width - 1 - x;
            y = ctx->height - 1 - y;
            break;
        case 3:
            _swap_uint16_t(x, y);
            y = ctx->height - 1 - y;
            break;
    }
    
    return ctx->buf[(y * ctx->width + x) / 8] & (1 << x) ? 1 : 0;
}


/// @brief Clears the screen
/// @param ctx Display context
void sharp_display_clear_display(struct sharp_display_context *ctx) {
    sharp_clear_display_buffer(ctx);
    
    uint8_t clear_data[2] = {(uint8_t)(ctx->vcom | SHARP_BIT_CLEAR),0x00};
    gpio_put(ctx->cs, 1);
    sharp_spi_transmit_seq(ctx->spi, clear_data, 2);
    // spi_write_blocking(ctx->spi, clear_data, 2);
    TOGGLE_VCOM(ctx->vcom);
    gpio_put(ctx->cs, 0);
}

/// @brief Render the contents of the buffer on the LCD
/// @param ctx Display context
void sharp_display_refresh(struct sharp_display_context *ctx) {
    uint16_t i, currentline;
    //uint8_t cmd;

    // CS
    gpio_put(ctx->cs, 1);

    // Send the write command
    //cmd = (ctx->vcom | SHARP_BIT_WRITECMD);
    TOGGLE_VCOM(ctx->vcom);
    //spi_write_blocking(ctx->spi, &cmd, 1);
    sharp_spi_transmit(ctx->spi, (ctx->vcom | SHARP_BIT_WRITECMD));
    
    
    uint8_t bytes_per_line = ctx->width / 8;
    uint16_t total_bytes = (ctx->width * ctx->height) / 8;
    
    for(i = 0; i < total_bytes; i += bytes_per_line) {
        uint8_t line[bytes_per_line + 2];
        
        // Send address byte
        currentline = ((i + 1) / (ctx->width / 8)) + 1;
        line[0] = currentline;
        // copy over this line
        memcpy(line + 1, ctx->buf + i, bytes_per_line);
        // add end of line
        line[bytes_per_line + 1] = 0x00;
        // send it
        //spi_write_blocking(ctx->spi, line, bytes_per_line + 2);
        sharp_spi_transmit_seq(ctx->spi, line, bytes_per_line + 2);
    }
    
    // send another trailing 8 bits for the last line
    //cmd = 0x00;
    //spi_write_blocking(ctx->spi, &cmd, 1);
    sharp_spi_transmit(ctx->spi, 0x00);
    
    gpio_put(ctx->cs, 0);

}

/***
**** Adafruit_GFX functions
***/

/// @brief Internal library function to write lines to buffer
/// @param ctx Display context
/// @param x0 Start point x coordinate
/// @param y0 Start point y coordinate
/// @param x1 End point x coordinate
/// @param y1 End point y coordinate
/// @param color Color
void sharp_write_line(struct sharp_display_context *ctx,
                              uint16_t x0,
                              int16_t y0, 
                              int16_t x1,
                              int16_t y1,
                              uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    }
    else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++)
    {
        if (steep) {
            sharp_display_draw_pixel(ctx, y0, x0, color);
        }
        else {
            sharp_display_draw_pixel(ctx, x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

/// @brief Draw a line
/// @param ctx Display context
/// @param x0 Start point x coordinate
/// @param y0 Start point y coordinate
/// @param x1 End point x coordinate
/// @param y1 End point y coordinate
/// @param color Color
void sharp_display_draw_line(struct sharp_display_context *ctx,
                              int16_t x0,
                              int16_t y0, 
                              int16_t x1,
                              int16_t y1,
                              uint16_t color) {
    if(x0 == x1) {
        if(y0 > y1)
            _swap_int16_t(y0, y1);
        sharp_display_draw_fast_v_line(ctx, x0, y0, y1 - y0 + 1, color);
    } else if(y0 == y1) {
        if(x0 > x1)
            _swap_int16_t(x0, x1);
        sharp_display_draw_fast_h_line(ctx, x0, y0, x1 - x0 + 1, color);
    } else {
        sharp_write_line(ctx, x0, y0, x1, y1, color);
    }
}

/// @brief Speed optimized vertical line drawing
/// @param ctx Display context
/// @param x Line horizontal start point
/// @param y Line vertical start point
/// @param h Length of vertical line to be drawn, including first point
/// @param color Color 
void sharp_display_draw_fast_v_line(struct sharp_display_context *ctx, int16_t x, int16_t y, int16_t h, uint16_t color) {
    sharp_write_line(ctx, x, y, x, y + h - 1, color);
}

/// @brief Speed optimized horizontal line drawing
/// @param ctx Display context
/// @param x Line horizontal start point
/// @param y Line vertical start point
/// @param w Length of horizontal line to be drawn, including first point
/// @param color Color 
void sharp_display_draw_fast_h_line(struct sharp_display_context *ctx, int16_t x, int16_t y, int16_t w, uint16_t color) {
    // Not actually speed optimized yet
    sharp_write_line(ctx, x, y, x + w - 1, y, color);
}

/// @brief Fill a rectangle completely with one color
/// @param ctx Display context
/// @param x Top left corner x coordinate
/// @param y Top left corner y coordinate
/// @param w Width in pixels
/// @param h Height in pixels
/// @param color Color
void sharp_display_fill_rect(struct sharp_display_context *ctx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    for(int16_t i = x; i < x + w; i++) {
        sharp_display_draw_fast_h_line(ctx, i, y, h, color);
    }
}

/// @brief Fill the screen completely with one color.
/// @param ctx Display context
/// @param color Color
void sharp_display_fill_screen(struct sharp_display_context *ctx, uint16_t color){
    sharp_display_fill_rect(ctx, 0, 0, ctx->width, ctx->height, color);
}