#ifndef FS_H
#define FS_H

#define MAX_FILES 10
#define MAX_FILE_SIZE 256

typedef struct {
    char name[16];
    char content[MAX_FILE_SIZE];
    int exists;
} File;

extern File fs[MAX_FILES];

void init_fs();

#endif