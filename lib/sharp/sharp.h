#pragma once

#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#define BLACK 0
#define WHITE 1

struct sharp_display_config {
    spi_inst_t *spi;
    uint cs;
    uint16_t width;
    uint16_t height;
    uint8_t rotation;
};

struct sharp_display_context {
    spi_inst_t *spi;
    const uint cs;
    const uint16_t width;
    const uint16_t height;
    uint8_t rotation;
    uint8_t * const buf;
    uint8_t vcom;
};

struct sharp_display_context *sharp_display_create_context(struct sharp_display_config *c);
void sharp_display_free_context(struct sharp_display_context *ctx);

void sharp_display_draw_pixel(struct sharp_display_context *ctx, int16_t x, int16_t y, uint16_t color);
uint8_t sharp_display_get_pixel(struct sharp_display_context *ctx, uint16_t x, uint16_t y);

void sharp_display_draw_line(struct sharp_display_context *ctx, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void sharp_display_draw_fast_v_line(struct sharp_display_context *ctx, int16_t x, int16_t y, int16_t h, uint16_t color);
void sharp_display_draw_fast_h_line(struct sharp_display_context *ctx, int16_t x, int16_t y, int16_t w, uint16_t color);

void sharp_display_fill_rect(struct sharp_display_context *ctx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void sharp_display_draw_rect(struct sharp_display_context *ctx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void sharp_display_fill_screen(struct sharp_display_context *ctx, uint16_t color);
void sharp_display_clear_display(struct sharp_display_context *ctx);
void sharp_display_refresh(struct sharp_display_context *ctx);