#ifndef FS_H
#define FS_H

#include <stdint.h>

#define CAWFS_MAGIC 0xCA705    // "CAWOS" magic number
#define MAX_FILES 16
#define FILES_START_LBA 71

typedef struct {
    uint32_t magic;
    uint32_t total_files;
} cawfs_superblock_t;

typedef struct {
    char name[32];
    uint32_t start_lba;
    uint32_t size_bytes;
    uint8_t  is_executable; // 1 для файлов в /bin
    uint8_t  exists;
} file_t;

// API
void fs_init();
void fs_list(int* row);
int  fs_load_to_memory(char* name, uint8_t* address);
int  fs_read_content(char* name, uint8_t* address);

#endif