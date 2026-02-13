#include "io.h"

// Чтение байта из порта
__attribute__((visibility("default")))
unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a" (result) : "dN" (port));
    return result;
}

// Запись байта в порт
__attribute__((visibility("default")))
void port_byte_out(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a" (data), "dN" (port));
}

// Запись слова (16 бит) в порт
__attribute__((visibility("default")))
void port_word_out(unsigned short port, unsigned short data) {
    __asm__ volatile("outw %0, %1" : : "a" (data), "dN" (port));
}

// Чтение слова (16 бит) из порта
__attribute__((visibility("default")))
unsigned short port_word_in(unsigned short port) {
    unsigned short result;
    __asm__ volatile("inw %1, %0" : "=a" (result) : "dN" (port));
    return result;
}