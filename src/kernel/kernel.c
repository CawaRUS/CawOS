#include "keyboard_map.h"
#include "io.h"
#include "util.h"
#include "screen.h"
#include "fs.h"
#include "idt.h"
#include "vga.h"
#include "gui.h"

// Внешние функции
extern void watchdog_reset();
extern void watchdog_check();
extern void vga_enter_mode13h();
extern unsigned short get_total_memory();
extern void itoa(int n, char str[]);
extern int strlen(char* s);

// Прототипы функций (ОБЯЗАТЕЛЬНО с точкой с запятой)
void draw_pixel(int x, int y, unsigned char color);
void execute_command(char* input, int* row);

// Оболочка GUI (теперь с телом функции)
extern void start_kgui();

void __main() {} // Пустая функция для некоторых линковщиков

void shutdown() {
    print_at_color("System Halting...", 24, 0, 0x0C);
    port_word_out(0x604, 0x2000);  // Выключение для QEMU
    while(1) { __asm__("hlt"); }
}

void main() {
    idt_init();
    init_fs();
    unsigned char* mode_ptr = (unsigned char*)0x449;
    if (*mode_ptr == 0x13) {
        start_kgui(); // Уходим в GUI и никогда не возвращаемся сюда
    }
    clear_screen();
    draw_logo();      
    beep();           
    
    clear_screen();
    print_at_color("CawOS v0.2 Just a OS.", 0, 0, 0x0B);
    print_at("Type 'help' to see all commands.", 1, 0);
    
    char key_buffer[256];
    int col = 2, row = 2, buffer_idx = 0;

    print_at("> ", row, 0);
    update_cursor(row, col);

    while(1) {
        watchdog_reset();

        if (port_byte_in(0x64) & 0x01) {
            unsigned char scancode = port_byte_in(0x60);
            
            if (scancode < 0x80) {
                if (scancode == ENTER) {
                    key_buffer[buffer_idx] = '\0';
                    row++; 
                    
                    if (buffer_idx > 0) {
                        execute_command(key_buffer, &row);
                        row++; 
                    }

                    buffer_idx = 0; 
                    col = 2;

                    if (row >= 22) {
                        clear_screen(); 
                        row = 0; 
                    }
                    
                    print_at("> ", row, 0);
                    update_cursor(row, col);
                } 
                else if (scancode == BACKSPACE) {
                    if (buffer_idx > 0) {
                        buffer_idx--; 
                        col--;
                        print_at(" ", row, col);
                        update_cursor(row, col);
                    }
                }
                else {
                    char key = ascii_map[scancode];
                    if (key != 0 && buffer_idx < 255) {
                        key_buffer[buffer_idx++] = key;
                        char str[2] = {key, 0};
                        print_at(str, row, col);
                        col++;
                        
                        if (col >= 78) { 
                            col = 2; 
                            row++; 
                            print_at("> ", row, 0); 
                        }
                        update_cursor(row, col);
                    }
                }
            }
        }
    }
}

void execute_command(char* input, int* row) {
    // 1. Команды ФС
    if (strcmp(input, "ls") == 0) {
        print_at("Files in memory:", *row, 0); 
        (*row)++;
        int count = 0;
        for (int i = 0; i < MAX_FILES; i++) {
            if (fs[i].exists) {
                print_at("- ", *row, 0); 
                print_at(fs[i].name, *row, 2);
                (*row)++; 
                count++;
            }
        }
        if (count == 0) {
            print_at("No files found.", *row, 0);
        } else {
            (*row)--; // Корректировка, так как main добавит еще одну строку
        }
        return;
    }
    
    if (strncmp(input, "touch ", 6) == 0) {
        char* fname = input + 6;
        for (int i = 0; i < MAX_FILES; i++) {
            if (!fs[i].exists) {
                strcpy(fs[i].name, fname);
                strcpy(fs[i].content, "(empty)");
                fs[i].exists = 1;
                print_at("File created.", *row, 0);
                return;
            }
        }
        print_at("Error: FS Full.", *row, 0);
        return;
    }
    
    if (strncmp(input, "write ", 6) == 0) {
        char* args = input + 6;
        char fname[16]; int i = 0;
        while (args[i] != ' ' && args[i] != '\0' && i < 15) {
            fname[i] = args[i]; i++;
        }
        fname[i] = '\0';
        for (int f = 0; f < MAX_FILES; f++) {
            if (fs[f].exists && strcmp(fs[f].name, fname) == 0) {
                if (args[i] == ' ') {
                    strcpy(fs[f].content, args + i + 1);
                }
                print_at("Success: Written.", *row, 0);
                return;
            }
        }
        print_at("Error: Not found.", *row, 0);
        return;
    }
    
    if (strncmp(input, "cat ", 4) == 0) {
        char* fname = input + 4;
        for (int i = 0; i < MAX_FILES; i++) {
            if (fs[i].exists && strcmp(fs[i].name, fname) == 0) {
                print_at_color(fs[i].content, *row, 0, 0x0E);
                return;
            }
        }
        print_at("Error: Not found.", *row, 0);
        return;
    }
    
    if (strncmp(input, "rm ", 3) == 0) {
        char* fname = input + 3;
        for (int i = 0; i < MAX_FILES; i++) {
            if (fs[i].exists && strcmp(fs[i].name, fname) == 0) {
                fs[i].exists = 0;
                print_at("File deleted.", *row, 0);
                return;
            }
        }
        return;
    }

    // 2. Системные команды
    if (strcmp(input, "help") == 0) {
        print_at("FS: ls, touch, write, cat, rm", *row, 0); 
        (*row)++;
        print_at("SYS: clear, logo, beep, info, reboot, shutdown", *row, 0);
        return;
    }
    
    if (strcmp(input, "info") == 0) {
        char vendor[13];
        char mem_str[16];
        get_cpu_info(vendor);
        
        // Получаем реальную память
        int mem_mb = get_total_memory() + 1; // +1МБ базовой памяти
        itoa(mem_mb, mem_str);

        print_at("CawOS v0.1.5", *row, 0); (*row)++;
        
        print_at("CPU: ", *row, 0); 
        print_at(vendor, *row, 5); (*row)++;
        
        print_at("RAM: ", *row, 0); 
        print_at(mem_str, *row, 5); 
        print_at(" MB", *row, 5 + strlen(mem_str)); (*row)++;
        
        // Для дисплея вытащим инфу о типе
        print_at("Display: VGA Color (80x25)", *row, 0); 
        return;
    }
    
    if (strcmp(input, "clear") == 0) { 
        clear_screen(); 
        *row = -1; 
        return; 
    }
    
    if (strcmp(input, "logo") == 0) { 
        draw_logo(); 
        clear_screen(); 
        *row = -1; 
        return; 
    }
    
    if (strcmp(input, "beep") == 0) { 
        beep(); 
        print_at("BEEEP!", *row, 0); 
        return; 
    }
    
    if (strcmp(input, "reboot") == 0) { 
        port_byte_out(0x64, 0xFE); 
        return; 
    }
    
    if (strcmp(input, "shutdown") == 0) { 
        shutdown(); 
        return; 
    }

    if (strcmp(input, "freeze") == 0) {
        print_at("System will freeze now...", *row, 0);
        while(1); // Имитируем зависание ядра
    }
    // Если ничего не подошло
    if (input[0] != '\0') {
        print_at_color("Unknown command!", *row, 0, 0x0C);
    }
}