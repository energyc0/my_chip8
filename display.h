#ifndef DISPLAY_H
#define DISPLAY_H

#include "chip-8.h"

//return coordinates for chip-8 display
#define WIDTH_TRUNC(x) ((x) & (CHIP8_DISPLAY_WIDTH-1))
#define HEIGHT_TRUNC(y) ((y) & (CHIP8_DISPLAY_HEIGHT - 1))

#define PIXEL '#'
#define BORDER '@'

void init_display(struct chip_8_internals* chip); //initialize ncurses
void cleanup_display(); //cleanup ncurses

//checks if terminal size is at least 64x32 and asks user to resize
void check_correct_display_size();

void clear_display(); //CLS instruction

byte_t draw_sprite(byte_t Vx, byte_t Vy, byte_t n); //DRW instruction, return VF value
#endif 