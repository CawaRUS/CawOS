#include "io.h"

// Чтение байта из порта
unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    // Используем "inb" для байта
    __asm__ volatile("inb %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

// Запись байта в порт
void port_byte_out(unsigned short port, unsigned char data) {
    // Используем "outb" для байта
    __asm__ volatile("outb %%al, %%dx" : : "a" (data), "d" (port));
}

// Запись слова (16 бит) в порт - ЭТОГО НЕ ХВАТАЛО
void port_word_out(unsigned short port, unsigned short data) {
    // Используем "outw" для слова (word)
    __asm__ volatile("outw %%ax, %%dx" : : "a" (data), "d" (port));
}