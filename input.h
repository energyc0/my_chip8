#ifndef CHIP8_INPUT_H
#define CHIP8_INPUT_H

#include "chip-8.h"

#define KEY_RELEASE_TIME_MS 150

//initialize user input
void init_input();

//key code is a hexadecimal number
//return 1 if key is pressed, 0 otherwise
int check_key(byte_t key_code);

//key code is a hexadecimal number
//wait till key is pressed and return scancode of it
byte_t get_key();

#endif