#include "fs.h"
#include "util.h"
#include "screen.h"
#include "commands.h"

// Явный прототип, чтобы убрать ошибку implicit declaration
extern int fs_read_content(char* name, unsigned char* address);

#ifndef NULL
#define NULL ((void*)0)
#endif

void cmd_cat(char* args, int* row) {
    if (args == NULL || args[0] == '\0') {
        print_at("Usage: cat <filename>", *row, 0);
        return;
    }

    static char file_buffer[2048]; 
    for(int i = 0; i < 2048; i++) file_buffer[i] = 0;

    if (fs_read_content(args, (unsigned char*)file_buffer)) {
        print_at_color(file_buffer, *row, 0, 0x0E);
        (*row)++; 
    } else {
        print_at_color("Error: File not found.", *row, 0, 0x0C);
    }
}

// ПЕРЕДАЕМ "cat" КАК СТРОКУ
REGISTER_COMMAND("cat", cmd_cat, 1);