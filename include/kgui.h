#ifndef KGUI_H
#define KGUI_H

// Функции из других модулей (asm или другие .c файлы)
extern void watchdog_reset();
extern void vga_enter_mode13h();
extern void gui_clear_screen(unsigned char color);
extern void gui_draw_window(int x, int y, int w, int h, char* title);
extern void draw_pixel(int x, int y, unsigned char color);

// Функции для работы с текстом в графике
void vga_draw_char(int x, int y, char c, unsigned char color);
void vga_print_string(int x, int y, const char* str, unsigned char color);

// Сама точка входа в GUI
void start_kgui();

#endif