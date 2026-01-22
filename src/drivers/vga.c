#include "io.h"
#include "font.h"

static const unsigned char mode_13h_settings[] = {
    /* MISC */ 0x63,
    /* SEQ */  0x03, 0x01, 0x0F, 0x00, 0x0E,
    /* CRTC */ 0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
               0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
               0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    /* GC */   0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    /* AC */   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
               0x41, 0x00, 0x0F, 0x00, 0x00
};

void vga_enter_mode13h() {
    int i;
    const unsigned char* regs = mode_13h_settings;

    // 1. ПОЛНАЯ РАЗБЛОКИРОВКА (подготовка)
    // Останавливаем Sequencer
    port_byte_out(0x3C4, 0x00);
    port_byte_out(0x3C5, 0x01);
    
    // Снимаем защиту записи с CRTC (бит 7 регистра 0x11)
    port_byte_out(0x3D4, 0x11);
    port_byte_out(0x3D5, port_byte_in(0x3D5) & ~0x80);

    // 2. ЗАПИСЬ MISC
    port_byte_out(0x3C2, *regs++);

    // 3. ЗАПИСЬ SEQUENCER
    for(i = 0; i < 5; i++) {
        port_byte_out(0x3C4, i);
        port_byte_out(0x3C5, *regs++);
    }

    // 4. ЗАПИСЬ CRTC
    // Форсированно снимаем защиту еще раз внутри цикла
    for(i = 0; i < 25; i++) {
        port_byte_out(0x3D4, i);
        unsigned char val = *regs++;
        if (i == 0x11) val &= ~0x80; 
        port_byte_out(0x3D5, val);
    }

    // 5. ЗАПИСЬ GRAPHICS CONTROLLER
    for(i = 0; i < 9; i++) {
        port_byte_out(0x3CE, i);
        port_byte_out(0x3CF, *regs++);
    }

    // 6. ЗАПИСЬ ATTRIBUTE CONTROLLER
    for(i = 0; i < 21; i++) {
        port_byte_in(0x3DA); // Сброс триггера AC (важно!)
        port_byte_out(0x3C0, i);
        port_byte_out(0x3C0, *regs++);
    }

    // 7. ФИНАЛИЗАЦИЯ
    port_byte_in(0x3DA);
    port_byte_out(0x3C0, 0x20); // Включаем палитру

    port_byte_out(0x3C4, 0x00); // Рестарт Sequencer
    port_byte_out(0x3C5, 0x03); 

    // 8. НАСТРОЙКА ПАЛИТРЫ DAC
    port_byte_out(0x3C8, 0x00);
    for (i = 0; i < 256; i++) {
        if (i == 0) {
            port_byte_out(0x3C9, 0); port_byte_out(0x3C9, 0); port_byte_out(0x3C9, 0);
        } else if (i == 1) { // Твой синий
            port_byte_out(0x3C9, 0); port_byte_out(0x3C9, 0); port_byte_out(0x3C9, 45);
        } else if (i == 2) { // Твой зеленый
            port_byte_out(0x3C9, 0); port_byte_out(0x3C9, 45); port_byte_out(0x3C9, 0);
        } else if (i == 7) { // Светло-серый (для тела окна)
            port_byte_out(0x3C9, 42); port_byte_out(0x3C9, 42); port_byte_out(0x3C9, 42);
        } else {
            // Стандартный градиент для остальных
            port_byte_out(0x3C9, i/4); port_byte_out(0x3C9, i/4); port_byte_out(0x3C9, i/4);
        }
    }
}


void draw_pixel(int x, int y, unsigned char color) {
    // Используем volatile, чтобы компилятор не хитрил с доступом к видеопамяти
    volatile unsigned char* screen = (volatile unsigned char*)0xA0000;
    
    // Проверка границ, чтобы не "вылететь" за пределы экрана и не уронить ядро
    if (x >= 0 && x < 320 && y >= 0 && y < 200) {
        screen[y * 320 + x] = color;
    }
}

// Рисует один символ в координатах x, y
void vga_draw_char(int x, int y, char c, unsigned char color) {
    for (int i = 0; i < 8; i++) {           // Проходим по 8 рядам (строкам) байта
        for (int j = 0; j < 8; j++) {       // Проходим по 8 битам в каждом ряду
            if (font8x8_basic[(int)c][i] & (1 << (7 - j))) {
                draw_pixel(x + j, y + i, color);
            }
        }
    }
}

// Рисует строку текста
void vga_print_string(int x, int y, const char* str, unsigned char color) {
    while (*str) {
        vga_draw_char(x, y, *str, color);
        x += 8; // Сдвигаемся на 8 пикселей вправо для следующей буквы
        str++;
    }
}