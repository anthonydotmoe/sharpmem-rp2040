#include <stdio.h>

#include "pico/stdlib.h"

#include "sharp.h"

// Used for printf-ing binary
const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

void print_byte(uint8_t byte) {
    printf("%s%s", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
}

int main() {
    stdio_init_all();
    
    sleep_ms(2000);
    
    // Initialize SPI
    gpio_set_function(19, GPIO_FUNC_SPI); // TX
    gpio_set_function(18, GPIO_FUNC_SPI); // CLK
    
    // Prevent SD Card from answering
    gpio_init(17);
    gpio_set_dir(17, GPIO_OUT);
    gpio_put(17, 1);

    struct sharp_display_context *display;
    struct sharp_display_config sharp_conf = {
        .spi = spi0,
        .cs = 27,
        .width = 400,
        .height = 240,
        .rotation = 0
    };
    
    display = sharp_display_create_context(&sharp_conf);
    
    sharp_display_clear_display(display);
    
    /*
    for(int i = 0; i < sharp_conf.width; i+= 2) {
        sharp_display_draw_pixel(display, i, 0, BLACK);
    }
    */

    for(int i = 0; i < sharp_conf.width; i += 4) {
        sharp_display_draw_line(display, 0, 0, i, sharp_conf.height - 1, 0);
        //sharp_display_refresh(display);
    }
    for(int i = 0; i < sharp_conf.height; i += 4) {
        sharp_display_draw_line(display, 0, 0, sharp_conf.width - 1, i, 0);
        //sharp_display_refresh(display);
    }

    sharp_display_refresh(display);
    
    sharp_display_free_context(display);
    
    while(1) {
        sleep_ms(1000);
    }
    
    return -1;
}
