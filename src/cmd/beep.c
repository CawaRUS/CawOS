#include "commands.h"
#include "screen.h"

extern void beep(); 

void cmd_beep(char* args, int* row) {
    (void)args; // Чтобы не было ворнингов
    beep();
    print_at("BEEEP!", *row, 0);
}

// ПЕРЕДАЕМ "beep" В КАВЫЧКАХ
REGISTER_COMMAND("beep", cmd_beep, 0);