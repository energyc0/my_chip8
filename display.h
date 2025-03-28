#ifndef DISPLAY_H
#define DISPLAY_H

#include "chip-8.h"

void clear_display(struct chip_8_internals* chip); //CLS instruction

byte_t draw_sprite(struct chip_8_internals* chip, byte_t x, byte_t y, byte_t n); //DRW instruction, return VF value
#endif 