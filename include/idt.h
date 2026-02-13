#ifndef IDT_H
#define IDT_H


#include <stdint.h>

struct idt_entry {
    unsigned short base_lo;
    unsigned short sel;        // Сегмент кода в GDT (у нас 0x08)
    unsigned char  always0;
    unsigned char  flags;      // Флаги доступа
    unsigned short base_hi;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

void idt_init();
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void watchdog_check();
void watchdog_reset();
extern volatile int watchdog_counter;

// Сюда будут прилетать все прерывания
struct registers {
    unsigned int ds;                  // Наш push eax
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha
    unsigned int int_no, err_code;    // Наши push byte
    unsigned int eip, cs, eflags, useresp, ss; // Пушит сам процессор
};
#endif