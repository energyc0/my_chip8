#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define CHIP8_RAM_SIZE 4096
#define CHIP8_REGISTERS_COUNT 16
#define CHIP8_STACK_SIZE 16
#define CHIP8_DISPLAY_WIDTH 64
#define CHIP8_DISPLAY_HEIGHT 32

typedef uint8_t byte_t;
typedef uint16_t word_t;

struct chip_8_internals{
    byte_t memory[CHIP8_RAM_SIZE];

    word_t I;
    word_t PC;
    byte_t SP;
    byte_t DT;
    byte_t ST;
    byte_t V[CHIP8_REGISTERS_COUNT];
    
    word_t stack[CHIP8_STACK_SIZE];
    byte_t display[CHIP8_DISPLAY_HEIGHT][CHIP8_DISPLAY_WIDTH];
};

void load_program(struct chip_8_internals* chip, char* program); //call on init
int fetch_inst(struct chip_8_internals* chip);
void decode_inst(struct chip_8_internals* chip);
void execute_inst(struct chip_8_internals* chip);
void cleanup();//call on exit

#endif