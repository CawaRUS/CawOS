#include "commands.h"
#include "fs.h"
#include "util.h"
#include "screen.h"

// 1. Снова extern, чтобы видеть таблицу файлов
extern file_t fs[MAX_FILES];

void cmd_touch(char* args, int* row) {
    // Проверка аргументов (args вместо fname)
    if (args == 0 || args[0] == '\0') {
        print_at("Usage: touch <filename>", *row, 0);
        (*row)++;
        return;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (!fs[i].exists) {
            // 2. Копируем имя из args
            strcpy(fs[i].name, args);
            
            // У новой ФС нет поля .content в структуре! 
            // Мы просто инициализируем метаданные:
            fs[i].size_bytes = 0;
            fs[i].start_lba = 0; // В идеале тут нужен аллокатор свободных блоков
            fs[i].is_executable = 0;
            fs[i].exists = 1;

            print_at("File created in table.", *row, 0);
            (*row)++;
            
            // TODO: Здесь нужно вызвать fs_flush() или ata_write28, 
            // чтобы сохранить таблицу на диск физически.
            
            return;
        }
    }
    print_at("Error: FS Full.", *row, 0);
    (*row)++;
}

// args_flag = 1, так как нам нужно имя файла
REGISTER_COMMAND("touch", cmd_touch, 1);