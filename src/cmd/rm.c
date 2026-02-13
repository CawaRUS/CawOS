#include "commands.h"
#include "fs.h"
#include "util.h"
#include "screen.h"

// 1. Объявляем внешний массив
extern file_t fs[MAX_FILES];

void cmd_rm(char* args, int* row) {
    // Проверка на пустой ввод
    if (args == 0 || args[0] == '\0') {
        print_at("Usage: rm <filename>", *row, 0);
        (*row)++;
        return;
    }

    // Используем args напрямую как имя файла
    for (int i = 0; i < MAX_FILES; i++) {
        // 2. Сравниваем fs[i].name с args
        if (fs[i].exists && strcmp(fs[i].name, args) == 0) {
            fs[i].exists = 0;
            print_at("File deleted from table.", *row, 0);
            (*row)++;
            return;
        }
    }
    
    print_at("Error: File not found.", *row, 0);
    (*row)++;
}

REGISTER_COMMAND("rm", cmd_rm, 1);