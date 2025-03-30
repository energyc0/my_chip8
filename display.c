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
    byte_t ret = 0;

    for(byte_t i = 0; i < n; i++){
        byte_t byte = chip->memory[chip->I + i];
        move(y+i,x);
        for (byte_t bit = 7; bit != (byte_t)-1; bit--) {
            byte_t symb = ((1 << bit) & byte);
            ret = (ret == 1) || (chip->display[WIDTH_TRUNC(x)][HEIGHT_TRUNC(y+i)] == 1 && symb == 1);
            chip->display[WIDTH_TRUNC(x)][HEIGHT_TRUNC(y+i)] ^= symb;
            addch(symb ? '#' : ' ');
        }
    }
    move(LINES-1, COLS-1);
    refresh();
    return ret;
}

void init_display(){
    initscr();
    noecho();
    cbreak();
}

void cleanup_display(){
    endwin();
}