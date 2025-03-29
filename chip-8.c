#include "chip-8.h"
#include "display.h"
#include "utils.h"
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

//current instruction
static word_t inst; 

//decoded instruction
static word_t nnn;  //the lowest 12 bits
static byte_t x;    //the lower 4 bits of high byte
static byte_t y;    //the upper 4 bits of low byte
static byte_t kk;   //the lowest 8 bits
static byte_t n;   //the lowest 4 bits

static void undefined_inst();

void load_program(struct chip_8_internals* chip, char* program){
    FILE* fp = fopen(program, "r");
    if(fp == NULL)
        print_err("Failed to open the file %s: %s\n", program, strerror(errno));    
    memset(chip, 0, sizeof(struct chip_8_internals));
        
    chip->PC = 0x200;
    chip->SP = (byte_t)-1;
    fseek(fp, 0, SEEK_END);
    size_t program_sz = ftell(fp);
    rewind(fp);

    fread(chip->memory+chip->PC, sizeof(byte_t), program_sz, fp);
    fclose(fp);

    srand(time(NULL));
    initscr();
    noecho();
    cbreak();
}

int fetch_inst(struct chip_8_internals* chip){
    if(chip->PC >= CHIP8_RAM_SIZE)
        return 0;
    byte_t first_byte = chip->memory[chip->PC];
    byte_t second_byte = chip->memory[chip->PC+1];

    inst = (first_byte << 8) + second_byte;
    chip->PC+=2;
    return inst != 0x0000;
}

void decode_inst(struct chip_8_internals* chip){
    nnn = (inst & 0x0FFF);
    n = (inst & 0x000F);
    x = ((inst & 0x0F00) >> 8);
    y = ((inst & 0x00F0) >> 4);
    kk = (inst & 0x00FF);
}

void execute_inst(struct chip_8_internals* chip){
   // fprintf(stderr,"%X%X%X%X\n",
    //    ((inst & 0xF000) >> 12), ((inst & 0x0F00) >> 8), ((inst & 0x00F0) >> 4), (inst & 0x000F));
    word_t temp_word;
    switch(((inst & 0xF000) >> 12)){
        case 0x0:{
            if(inst == 0x00E0){
                clear_display(chip);
                break;
            }else if(inst == 0x00EE){
                if(chip->SP == (byte_t)-1)
                    exit(EXIT_SUCCESS);
                chip->PC = chip->stack[chip->SP--];
                break;
            }
            undefined_inst(); break;
        }
        case 0x1: chip->PC = nnn; break;
        case 0x2: {
            if(++chip->SP >= CHIP8_STACK_SIZE)
                print_err("CHIP8 stackoverflow\n");
            chip->stack[chip->SP--] = chip->PC;
            chip->PC = nnn; break;
        }
        case 0x3:{
            if(chip->V[x] == kk)
                chip->PC +=2;
            break;
        }
        case 0x4:{
            if(chip->V[x] != kk)
                chip->PC +=2;
            break;
        }
        case 0x5:{
            if((inst & 0x000F) != 0)
                undefined_inst();
            if(chip->V[x] == chip->V[y])
                chip->PC +=2;
            break;
        }
        case 0x6: chip->V[x] = kk; break;
        case 0x7: chip->V[x] += kk; break;
        case 0x8: {
            switch (inst & 0x000F) {
                case 0x0: chip->V[x] |= chip->V[y]; break;
                case 0x1: chip->V[x] |= chip->V[y]; break;
                case 0x2: chip->V[x] &= chip->V[y]; break;
                case 0x3: chip->V[x] ^= chip->V[y]; break;
                case 0x4:{
                    temp_word = chip->V[x] + chip->V[y];
                    chip->V[0xF] = temp_word > 0xFF;
                    chip->V[x] = temp_word;
                    break;
                }
                case 0x5:{
                    chip->V[0xF] = chip->V[x] > chip->V[y];
                    chip->V[x] -= chip->V[y];
                    break;
                }
                case 0x6:{
                    chip->V[0xF] = chip->V[x] & 0x01;
                    chip->V[x] /= 2;
                    break;
                }
                case 0x7:{
                    chip->V[0xF] = chip->V[y] > chip->V[x];
                    chip->V[x] = chip->V[y] - chip->V[x];
                    break;
                }
                case 0xE:{
                    chip->V[0xF] = chip->V[x] & 0x80;
                    chip->V[x] *= 2;
                    break;
                }
                default: undefined_inst();
            }
            break;
        }
        case 0x9:{
            if((inst & 0x000F) != 0)
                undefined_inst();
            if(chip->V[x] != chip->V[y])
                chip->PC+=2;
            break;
        }
        case 0xA: chip->I = nnn; break;
        case 0xB: chip->PC = nnn + chip->V[0x0]; break;
        case 0xC: chip->V[x] = (rand() % 256) & kk; break;
        case 0xD: chip->V[0xF] = draw_sprite(chip, chip->V[x], chip->V[y], n); break;
        case 0xE: undefined_inst(); break; //TODO
        case 0xF:{
            switch (inst & 0x00FF) {
                case 0x07: chip->V[x] = chip->DT; break;
                case 0x0A: undefined_inst(); break; //TODO
                case 0x15: chip->DT = chip->V[x]; break;
                case 0x18: undefined_inst(); break; //TODO
                case 0x1E: undefined_inst(); break; //TODO
                case 0x29: undefined_inst(); break; //TODO
                case 0x33: undefined_inst(); break; //TODO
                case 0x55: undefined_inst(); break; //TODO
                case 0x65: undefined_inst(); break; //TODO
                default: undefined_inst();
            }
        }
        default: undefined_inst();
    }
}

static void undefined_inst(){
    print_err("Undefined instruction: %X%X%X%X\n",
         ((inst & 0xF000) >> 12), ((inst & 0x0F00) >> 8), ((inst & 0x00F0) >> 4), (inst & 0x000F));
}