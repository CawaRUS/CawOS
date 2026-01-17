#include "fs.h"

File fs[MAX_FILES];

void init_fs() {
    for (int i = 0; i < MAX_FILES; i++) fs[i].exists = 0;
}