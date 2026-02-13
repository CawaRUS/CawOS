#include "commands.h"
#include "util.h"
#include "screen.h"

// Вот этот парень спасет компиляцию:
#ifndef NULL
#define NULL ((void*)0)
#endif

// Эти метки нам даст линковщик из файла .ld
extern command_t __start_cmd;
extern command_t __stop_cmd;

void execute_command(char* input, int* row) {
    if (input[0] == '\0') return;

    command_t* cmd;
    for (cmd = &__start_cmd; cmd < &__stop_cmd; cmd++) {
        int name_len = strlen(cmd->name);

        if (cmd->has_args) {
            // Проверка на пробел или конец строки для команд с аргументами
            if (strncmp(input, cmd->name, name_len) == 0 && (input[name_len] == ' ' || input[name_len] == '\0')) {
                char* args = (input[name_len] == ' ') ? (input + name_len + 1) : "";
                cmd->func(args, row);
                return;
            }
        } else {
            if (strcmp(input, cmd->name) == 0) {
                cmd->func(NULL, row); // Теперь NULL определен!
                return;
            }
        }
    }
    print_at_color("Unknown command!", *row, 0, 0x0C);
    (*row)++;
}