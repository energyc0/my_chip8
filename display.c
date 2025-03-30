#include "display.h"
#include "chip-8.h"
#include "utils.h"
#include <ncurses.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#define MIN_TERM_WIDTH (CHIP8_DISPLAY_WIDTH + 4)
#define MIN_TERM_HEIGHT (CHIP8_DISPLAY_HEIGHT + 5)

//CHIP-8 screen upper-left corner coordinates in the terminal
static int start_x = 0;
static int start_y = 0;
static struct chip_8_internals* pchip = NULL;

//clear and draw chip-8 display
static void redraw_display(const byte_t display[CHIP8_DISPLAY_HEIGHT][CHIP8_DISPLAY_WIDTH]);

static void resize_handler(int code);

void clear_display(){
    memset(pchip->display, 0, CHIP8_DISPLAY_HEIGHT * CHIP8_DISPLAY_WIDTH);
    clear();
    refresh();
}

byte_t draw_sprite(byte_t x, byte_t y, byte_t n){
    byte_t ret = 0;

    for(byte_t i = 0; i < n; i++){
        byte_t byte = pchip->memory[pchip->I + i];
        move(y+i,x);
        for (byte_t bit = 7; bit != (byte_t)-1; bit--) {
            byte_t symb = ((1 << bit) & byte);
            ret = (ret == 1) || (pchip->display[HEIGHT_TRUNC(y+i)][WIDTH_TRUNC(x)] == 1 && symb == 1);
            pchip->display[HEIGHT_TRUNC(y+i)][WIDTH_TRUNC(x)] ^= symb;
            addch(symb ? PIXEL : ' ');
        }
    }
    move(LINES-1, COLS-1);
    refresh();
    return ret;
}

void init_display(struct chip_8_internals* chip){
    pchip = chip;

    initscr();
    noecho();
    cbreak();

    struct sigaction sgnl;
    memset(&sgnl, 0, sizeof(sgnl));
    sgnl.sa_flags = SA_RESTART | SA_NODEFER;
    sgnl.sa_handler = resize_handler;

    if(sigaction(SIGWINCH, &sgnl, NULL) == -1)
        eprintf("sigaction(): %s\n", strerror(errno));
}

void cleanup_display(){
    endwin();
}

int check_correct_display_size(){
    while(!(COLS >= MIN_TERM_WIDTH && LINES >= MIN_TERM_HEIGHT)){
        mvprintw(0, 0, "Current terminal size is %dx%d\nTerminal size should be at least %dx%d ", LINES, COLS, MIN_TERM_WIDTH, MIN_TERM_HEIGHT);
        move(LINES-1, COLS-1);
        refresh();

        pause();

        move(0,0);
        clrtoeol();
        move(1,0);
        clrtoeol();
    }


    return 1;
}

static void redraw_display(const byte_t display[CHIP8_DISPLAY_HEIGHT][CHIP8_DISPLAY_WIDTH]){
    clear();
    char line[CHIP8_DISPLAY_WIDTH + 1];
    line[CHIP8_DISPLAY_WIDTH] = '\0';

    for(byte_t y = 0; y < CHIP8_DISPLAY_HEIGHT; y++){
        for(byte_t x = 0; x < CHIP8_DISPLAY_WIDTH; x++){
            line[x] = display[y][x] ? PIXEL : ' ';
        }
        mvaddstr(y, 0, line);
    }

    refresh();
}

static void resize_handler(int code){
    static WINDOW* saved_scr = NULL;
    static int lines = -1;
    static int cols = -1;
    if(saved_scr == NULL){
        lines = LINES; cols = COLS;
        saved_scr = newwin(LINES,COLS,0,0);
        copywin(stdscr, saved_scr, 0, 0, 0, 0, LINES-1, COLS-1, 0);
    }
    endwin();
    initscr();
    
    check_correct_display_size();
    lines = lines > LINES ? LINES : lines;
    cols = cols > COLS ? COLS : cols;
    copywin(saved_scr, stdscr, 0, 0, 0, 0, lines-1, cols-1, 0);
    delwin(saved_scr);
    saved_scr = NULL;
    refresh();
}