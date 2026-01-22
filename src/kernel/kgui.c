#include "vga.h"
#include "gui.h"
#include "font.h"
#include "kgui.h"
#include "io.h" // Для port_byte_in

// Глобальные координаты мыши
int mouse_x = 160;
int mouse_y = 100;

// Буфер экрана (320 * 200 = 64000 байт)
// Позволяет сначала рисовать всё в памяти, а потом мгновенно выводить на экран
unsigned char screen_buffer[64000];

void draw_mouse_cursor(int x, int y, unsigned char color) {
    // Рисуем маленькую стрелочку
    draw_pixel(x, y, color);
    draw_pixel(x+1, y, color);
    draw_pixel(x, y+1, color);
    draw_pixel(x+1, y+1, color);
    draw_pixel(x+2, y+2, color);
}

void render_ui() {
    // 1. Фон (зеленый)
    gui_clear_screen(2); 

    // 2. Панель задач
    for(int x = 0; x < 320; x++) {
        for(int y = 180; y < 200; y++) {
            draw_pixel(x, y, 7); 
        }
    }

    // 3. Текст на панели
    vga_print_string(5, 187, "Start", 15);
    vga_print_string(250, 187, "12:00", 0);

    // 4. Окно приветствия
    gui_draw_window(60, 40, 200, 100, "CawOS 0.2");
    vga_print_string(70, 70, "Welcome to CawOS!", 0);
    vga_print_string(70, 85, "Just a OS!", 8);
    vga_print_string(70, 95, "WARNING:", 8);
    vga_print_string(70, 105, "ITS THE TEST!", 8);
    vga_print_string(70, 115, "BUGS = 100%!!!", 8);
    
    // 5. Курсор мыши (рисуется в последнюю очередь, поверх всего)
    draw_mouse_cursor(mouse_x, mouse_y, 15); // Белый курсор
}

void start_kgui() {
    // Первоначальная отрисовка
    render_ui();

    while(1) {
        watchdog_reset();

        // Простой опрос клавиатуры (если нажат ESC - выходим в консоль)
        if (port_byte_in(0x64) & 0x01) {
            unsigned char scancode = port_byte_in(0x60);
            if (scancode == 0x01) return; 

            // Управление курсором стрелками (пока нет драйвера мыши)
            if (scancode == 0x4B) { mouse_x -= 4; render_ui(); } // Влево
            if (scancode == 0x4D) { mouse_x += 4; render_ui(); } // Вправо
            if (scancode == 0x48) { mouse_y -= 4; render_ui(); } // Вверх
            if (scancode == 0x50) { mouse_y += 4; render_ui(); } // Вниз
        }

        // Ограничиваем курсор рамками экрана
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_x > 315) mouse_x = 315;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_y > 195) mouse_y = 195;
    }
}