#include "commands.h"
#include "fs.h"
#include "screen.h"
#include "util.h"

extern file_t fs[MAX_FILES];

void cmd_ls(char* args, int* row) {
    (void)args; 
    print_at("Files on disk (CawFS):", *row, 0); 
    (*row)++;
    
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs[i].exists) {
            print_at("- ", *row, 0); 
            print_at(fs[i].name, *row, 2);
            
            char s_buf[16];
            // ИСПРАВЛЕНО: убрали ", 10", так как твоя itoa принимает 2 аргумента
            itoa(fs[i].size_bytes, s_buf); 
            
            print_at(s_buf, *row, 20);
            print_at(" bytes", *row, 20 + strlen(s_buf));
            
            (*row)++; 
            count++;
        }
    }
    
    if (count == 0) {
        print_at("No files found.", *row, 0);
        (*row)++;
    }
}

REGISTER_COMMAND("ls", cmd_ls, 0);