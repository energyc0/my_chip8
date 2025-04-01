#include "input.h"
#include "chip-8.h"
#include "utils.h"
#include <ncurses.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/time.h>

//user keyboard, every key is a hexadecimal number
//stores last time key pressed
static struct timeval keyboard[16];
//mutex for keyboard access
static pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
//condition for get_key() function
static pthread_cond_t input_cond = PTHREAD_COND_INITIALIZER;
//last read char from stdin
static int ch = '\0';

//function for input routine
static void input_thread();

//convert char to chip-8 key or return -1(255)
static byte_t char_to_scancode(int ch);

void init_input(){
    pthread_t thr;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    
    pthread_create(&thr, &attr, (void*)input_thread, NULL);
    pthread_attr_destroy(&attr);
}

int check_key(byte_t key_code){
    pthread_mutex_lock(&input_mutex);
    int ret = diff_cur_time(&keyboard[key_code & 0x0F]) < KEY_RELEASE_TIME_MS;
    pthread_mutex_unlock(&input_mutex);
    return ret;
}

byte_t get_key(){
    byte_t scancode = 0xFF;
    pthread_mutex_lock(&input_mutex);

    while (scancode != 0xFF){
        pthread_cond_wait(&input_cond, &input_mutex); //wait input from input_thread()
        byte_t scancode = char_to_scancode(ch);
    }

    pthread_mutex_unlock(&input_mutex);
    return scancode;
}

static void input_thread(){
    while ((ch = getch()) != EOF) {
        pthread_cond_signal(&input_cond); //signal main thread if it is in get_key() function

        pthread_mutex_lock(&input_mutex);

        byte_t scancode = char_to_scancode(ch);
        if(scancode != 0xFF)
            gettimeofday(&keyboard[scancode], NULL);

        pthread_mutex_unlock(&input_mutex);
    }
}

static byte_t char_to_scancode(int ch){
    switch (tolower(ch)) {
        case '1': return 0x1;
        case '2': return 0x2;
        case '3': return 0x3;
        case '4': return 0xC;
        case 'q': return 0x4;
        case 'w': return 0x5;
        case 'e': return 0x6;
        case 'r': return 0xD;
        case 'a': return 0x7;
        case 's': return 0x8;
        case 'd': return 0x9;
        case 'f': return 0xE;
        case 'z': return 0xA;
        case 'x': return 0x0;
        case 'c': return 0xB;
        case 'v': return 0xF;
        default: return 0xFF;
    }
}