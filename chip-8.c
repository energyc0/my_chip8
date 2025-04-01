#include "chip-8.h"
#include "display.h"
#include "utils.h"
#include "input.h"
#include <bits/pthreadtypes.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

//current instruction
static word_t inst; 

//decoded instruction
static word_t nnn;  //the lowest 12 bits
static byte_t x;    //the lower 4 bits of high byte
static byte_t y;    //the upper 4 bits of low byte
static byte_t kk;   //the lowest 8 bits
static byte_t n;   //the lowest 4 bits

static void init_font(byte_t* memory);  //initialize chip-8 memory 
static byte_t hex_char_addr(byte_t Vx); //return address offset to the hexadecimal character in chip-8 memory 
static void bin_cod_dec_conv(struct chip_8_internals* chip, byte_t Vx); //binary-coded decimal conversion

//print error and exit
#define undefined_inst()     eprintf("Undefined instruction: %X%X%X%X\n",   \
    ((inst & 0xF000) >> 12), ((inst & 0x0F00) >> 8), ((inst & 0x00F0) >> 4), (inst & 0x000F)) \

//for timer thread
struct chip8_timer_setup{
    byte_t* DT;   //delay timer
    byte_t* ST;   //sound timer
    pthread_barrier_t* setup_barrier;
};
//must call on init
static void init_timers(struct chip_8_internals* chip);
//thread decreasing the delay and sound timers
static void timer_thread(const struct chip8_timer_setup* t);
//mutex for timers access
static pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;

void load_program(struct chip_8_internals* chip, char* program){
    FILE* fp = fopen(program, "r");
    if(fp == NULL)
        eprintf("Failed to open the file %s: %s\n", program, strerror(errno));    
    
    //initialize chip-8
    memset(chip, 0, sizeof(struct chip_8_internals));
    chip->PC = 0x200;
    chip->SP = (byte_t)-1;
    init_font(chip->memory);


    fseek(fp, 0, SEEK_END);
    size_t program_sz = ftell(fp);
    rewind(fp);

    fread(chip->memory+chip->PC, 1, program_sz, fp);
    fclose(fp);

    srand(time(NULL));
    init_display(chip);
    check_correct_display_size();
    init_input();
    init_timers(chip);
    atexit(cleanup);
}

void cleanup(){
    cleanup_display();
}

int fetch_inst(struct chip_8_internals* chip){
    static struct timeval prev_time;
    while (diff_cur_time(&prev_time) < 1000.0/CHIP8_INSTRUCTIONS_PER_SEC);
    gettimeofday(&prev_time, NULL);

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
    word_t temp_word;
    mvprintw(0, 0, "Current instruction: %X%X%X%X",
        ((inst & 0xF000) >> 12), ((inst & 0x0F00) >> 8), ((inst & 0x00F0) >> 4), (inst & 0x000F));

    switch(((inst & 0xF000) >> 12)){
        case 0x0:{
            if(inst == 0x00E0){
                clear_display();
            }else if(inst == 0x00EE){
                if(chip->SP == (byte_t)-1)
                    eprintf("Dalboeb\n");//exit(EXIT_SUCCESS);
                chip->PC = chip->stack[chip->SP--];
            }
            break;
        }
        case 0x1: chip->PC = nnn; break;
        case 0x2: {
            if(++(chip->SP) >= CHIP8_STACK_SIZE)
                eprintf("CHIP8 stackoverflow\n");
            chip->stack[chip->SP] = chip->PC;
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
                case 0x0: chip->V[x] = chip->V[y]; break;
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
                case 0x6:{ //AMBIGUOUS
                    chip->V[0xF] = chip->V[x] & 0x01;
                    chip->V[x] /= 2;
                    break;
                }
                case 0x7:{
                    chip->V[0xF] = chip->V[y] > chip->V[x];
                    chip->V[x] = chip->V[y] - chip->V[x];
                    break;
                }
                case 0xE:{ //AMBIGUOUS
                    chip->V[0xF] = chip->V[x] & 0x80;
                    chip->V[x] <<= 1;
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
        case 0xB: chip->PC = nnn + chip->V[0x0]; break; //AMBIGUOUS
        case 0xC: chip->V[x] = (rand() % 256) & kk; break;
        case 0xD: chip->V[0xF] = draw_sprite(chip->V[x], chip->V[y], n); break;
        case 0xE:{
            if((inst & 0x00FF) ==  0x009E){
                if(check_key(chip->V[x]))
                    chip->PC += 2;
            }else if((inst & 0x00FF) == 0x00A1){
                if(!check_key(chip->V[x]))
                    chip->PC += 2;
            }else
                undefined_inst();
            break;
        }
        case 0xF:{
            switch (inst & 0x00FF) {
                case 0x07:{
                    pthread_mutex_lock(&timer_mutex);
                    chip->V[x] = chip->DT;
                    pthread_mutex_unlock(&timer_mutex);
                    break;
                }
                case 0x0A: chip->V[x] = get_key(); break;
                case 0x15:{
                    pthread_mutex_lock(&timer_mutex);
                    chip->DT = chip->V[x];
                    pthread_mutex_unlock(&timer_mutex);
                    break;
                }
                case 0x18:{
                    pthread_mutex_lock(&timer_mutex);
                    chip->ST = chip->V[x];
                    pthread_mutex_unlock(&timer_mutex);
                    break;
                }
                case 0x1E: chip->I += chip->V[x]; break;
                case 0x29: chip->I = hex_char_addr(chip->V[x]); break;
                case 0x33: bin_cod_dec_conv(chip, chip->V[x]); break;
                case 0x55:{
                    if(chip->I + x >= CHIP8_RAM_SIZE)
                        eprintf("access CHIP-8 memory out of range\n");
                    memcpy(chip->memory + chip->I, chip->V, x+1);
                    break;
                }
                case 0x65:{ //AMBIGUOUS
                    if(chip->I + x >= CHIP8_RAM_SIZE)
                        eprintf("access CHIP-8 memory out of range\n");
                    memcpy(chip->V, chip->memory, x+1);
                    break;
                }
                default: undefined_inst();
            }
            break;
        }
        default: undefined_inst();
    }
}

static void init_font(byte_t* memory){
    const byte_t font_digits[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    memcpy(memory, font_digits, sizeof(font_digits));
}

static byte_t hex_char_addr(byte_t Vx){
    return (Vx & 0x0F) * 5;
}

static void bin_cod_dec_conv(struct chip_8_internals* chip, byte_t Vx){
    if(chip->I + 2 >= CHIP8_RAM_SIZE)
        eprintf("access CHIP-8 memory out of range\n");
    chip->memory[chip->I + 2] = Vx % 10;
    Vx /= 10;
    chip->memory[chip->I + 1] = Vx % 10;
    Vx /= 10;
    chip->memory[chip->I] = Vx;
}

static void init_timers(struct chip_8_internals* chip){
    pthread_t thr;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, 2);

    struct chip8_timer_setup setup;
    setup.DT = &chip->DT;
    setup.ST = &chip->ST;
    setup.setup_barrier = &barrier;

    pthread_create(&thr, &attr, (void*)timer_thread, (void*)&setup);
    pthread_barrier_wait(&barrier);

    pthread_barrier_destroy(&barrier);
    pthread_attr_destroy(&attr);
}

static void timer_thread(const struct chip8_timer_setup* t){
    byte_t* ST = t->ST;
    byte_t* DT = t->DT;
    pthread_barrier_wait(t->setup_barrier);

    while (1) {
        usleep(1e6 / CHIP8_TIMER_DECREASE_PER_SEC);
        pthread_mutex_lock(&timer_mutex);
        if(*ST > 0){
            if(--(*ST) == 0)
                beep();
        }
        if(*DT > 0) (*DT)--;
        pthread_mutex_unlock(&timer_mutex);
    }
}