#include "vga.h"

void gui_clear_screen(unsigned char color) {
    volatile unsigned char* vga = (volatile unsigned char*)0xA0000;
    for (int i = 0; i < 320 * 200; i++) {
        vga[i] = color;
    }
}

void gui_draw_rect(int x, int y, int w, int h, unsigned char color) {
    // Жёсткая проверка: если прямоугольник выходит за экран, мы его обрезаем
    for (int i = 0; i < h; i++) {
        int screen_y = y + i;
        if (screen_y < 0 || screen_y >= 200) continue;

        for (int j = 0; j < w; j++) {
            int screen_x = x + j;
            if (screen_x < 0 || screen_x >= 320) continue;

            draw_pixel(screen_x, screen_y, color);
        }
    }
}

void gui_draw_window(int x, int y, int w, int h, char* title) {
    // 1. Тело окна (Серый - 7)
    gui_draw_rect(x, y, w, h, 7);
    
    // 2. Заголовок (Синий - 1)
    // Ограничиваем высоту заголовка жестко 12 пикселями
    gui_draw_rect(x, y, w, 12, 1);
    
    // 3. Рамка (Черный - 0)
    for (int i = 0; i < w; i++) {
        draw_pixel(x + i, y, 0);         // Верх
        draw_pixel(x + i, y + h - 1, 0); // Низ
    }
    for (int i = 0; i < h; i++) {
        draw_pixel(x, y + i, 0);         // Лево
        draw_pixel(x + w - 1, y + i, 0); // Право
    }
}