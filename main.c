#include "utils.h"
#include "chip-8.h"

int main(int argc, char** argv){
    if(argc != 2)
        eprintf("CHIP-8 rom filename expected\n");

    struct chip_8_internals chip8;
    load_program(&chip8, argv[1]);
    while(fetch_inst(&chip8)){
        decode_inst(&chip8);
        execute_inst(&chip8);
    }
    cleanup();
    return 0;
}