#include "commands.h"
#include "fs.h"
#include "util.h"
#include "screen.h"
#include "ata.h" // Нужен для ata_write28

extern file_t fs[MAX_FILES];
extern ata_device_t* main_dev; // Если он объявлен глобально в fs.c или ata.c

void cmd_write(char* args, int* row) {
    if (args == 0 || args[0] == '\0') {
        print_at("Usage: write <file> <text>", *row, 0);
        (*row)++; return;
    }

    char fname[32]; int i = 0;
    while (args[i] != ' ' && args[i] != '\0' && i < 31) {
        fname[i] = args[i]; i++;
    }
    fname[i] = '\0';

    for (int f = 0; f < MAX_FILES; f++) {
        if (fs[f].exists && strcmp(fs[f].name, fname) == 0) {
            if (args[i] == ' ') {
                char* text = args + i + 1;
                uint32_t len = strlen(text);
                
                // 1. Обновляем размер в таблице файлов
                fs[f].size_bytes = len;

                // 2. Пишем данные в LBA файла (минимум 1 сектор)
                // ВАЖНО: Мы предполагаем, что у файла уже есть назначенный start_lba
                uint32_t sectors = (len + 511) / 512;
                if (fs[f].start_lba >= 71) { // Защита, чтобы не затереть ядро
                    ata_write28(main_dev, fs[f].start_lba, sectors, (uint8_t*)text);
                    
                    // 3. Сохраняем обновленную таблицу файлов на диск (LBA 65)
                    ata_write28(main_dev, 65, 2, (uint8_t*)fs);
                    
                    print_at("Success: Written to disk.", *row, 0);
                } else {
                    print_at("Error: Invalid file LBA.", *row, 0);
                }
            } else {
                print_at("Error: No text provided.", *row, 0);
            }
            (*row)++; return;
        }
    }
    print_at("Error: File not found.", *row, 0);
    (*row)++;
}

REGISTER_COMMAND("write", cmd_write, 1);