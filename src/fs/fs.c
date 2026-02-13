#include "fs.h"
#include "ata.h"
#include "util.h"
#include "screen.h"

file_t fs[MAX_FILES]; 
ata_device_t* main_dev;

void fs_init() {
    extern ata_device_t ata_devices[]; 
    main_dev = &ata_devices[0]; 
    ata_read28(main_dev, 65, 2, (uint8_t*)fs);
}

// Теперь функция принимает указатель на строку row
void fs_list(int* row) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs[i].exists) {
            // Печатаем всё в одну строку row, разделяя по колонкам (col)
            print_at("File: ", *row, 0);
            print_at(fs[i].name, *row, 6);
            print_at("| Size: ", *row, 22);
            
            char s_buf[16];
            itoa(fs[i].size_bytes, s_buf);
            print_at(s_buf, *row, 30);
            print_at(" bytes", *row, 30 + strlen(s_buf));
            
            // После каждого файла увеличиваем номер строки
            (*row)++; 
        }
    }
}

int fs_load_to_memory(char* name, uint8_t* address) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs[i].exists && strcmp(fs[i].name, name) == 0) {
            uint32_t sectors = (fs[i].size_bytes / 512) + 1;
            ata_read28(main_dev, fs[i].start_lba, sectors, address);
            return 1;
        }
    }
    return 0;
}

int fs_read_content(char* name, uint8_t* address) {
    return fs_load_to_memory(name, address);
}