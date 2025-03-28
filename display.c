#include "display.h"
#include "chip-8.h"
#include <ncurses.h>
#include <string.h>

void clear_display(struct chip_8_internals* chip){
    memset(chip->display, 0, CHIP8_DISPLAY_HEIGHT * CHIP8_DISPLAY_WIDTH);
    clear();
    refresh();
}

byte_t draw_sprite(struct chip_8_internals* chip, byte_t x, byte_t y, byte_t n){
    byte_t buf[9];
    byte_t ret = 0;
    buf[8] = '\0';

    //print_err("x = %d, y = %d, n = %d\n", x,y,n);

    for(byte_t i = 0; i < n; i++){
        byte_t byte = chip->memory[chip->I + i];
        ret = ((ret == 1) || (byte ^ chip->display[chip->I + i]));
        for(byte_t bit = 0; bit < 8; bit++){
            buf[bit] = (byte & (1 << bit)) ? '#' : ' ';
        }
        mvaddstr(y+i, x, buf);
    }
    move(LINES-1, COLS-1);
    refresh();
    return ret;
}