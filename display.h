#ifndef DISPLAY_H
#define DISPLAY_H

#include "chip-8.h"

//return coordinates for chip-8 display
#define WIDTH_TRUNC(x) ((x) & (CHIP8_DISPLAY_WIDTH-1))
#define HEIGHT_TRUNC(y) ((y) & (CHIP8_DISPLAY_HEIGHT - 1))

void init_display(); //initialize ncurses
void cleanup_display(); //cleanup ncurses

void clear_display(struct chip_8_internals* chip); //CLS instruction

byte_t draw_sprite(struct chip_8_internals* chip, byte_t Vx, byte_t Vy, byte_t n); //DRW instruction, return VF value
#endif 