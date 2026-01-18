#include "idt.h"
#include "util.h"
#include "screen.h"
#include "io.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void isr0();
extern void idt_load(unsigned int);
extern void isr32();

volatile int watchdog_counter = 0;
const int WATCHDOG_LIMIT = 1000; 

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void draw_panic_screen(const char* msg) {
    unsigned char speaker_state = port_byte_in(0x61);
    port_byte_out(0x61, speaker_state & 0xFC);
    char* vm = (char*)0xb8000;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vm[i] = ' ';
        vm[i+1] = 0x4F; 
    }
    int offset = (12 * 80 + 20) * 2;
    for (int i = 0; msg[i] != '\0'; i++) {
        vm[offset + (i * 2)] = msg[i];
        vm[offset + (i * 2) + 1] = 0x4F;
    }
}

void isr_handler(struct registers r) {

    if (r.int_no < 32) {
        if (r.int_no == 0) draw_panic_screen(" KERNEL PANIC: DIVISION BY ZERO ");
        else draw_panic_screen(" KERNEL PANIC: CPU EXCEPTION ");
        while(1);
    }

    if (r.int_no == 32) {
        static int tick_visual = 0;

        watchdog_check();
        port_byte_out(0x20, 0x20); // EOI
    }
}

void init_timer(int frequency) {
    int divisor = 1193180 / frequency;
    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, divisor & 0xFF);
    port_byte_out(0x40, (divisor >> 8) & 0xFF);
}

void idt_init() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    // Настройка PIC
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);
    port_byte_out(0x21, 0x20); 
    port_byte_out(0xA1, 0x28);
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);

    // Маскировка: только IRQ0
    port_byte_out(0x21, 0xFE); 
    port_byte_out(0xA1, 0xFF);

    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    idt_set_gate(0, (unsigned int)isr0, 0x08, 0x8E);
    idt_set_gate(32, (unsigned int)isr32, 0x08, 0x8E);

    idt_load((unsigned int)&idtp);

    init_timer(100);

    __asm__ __volatile__("sti");
}

void watchdog_check() {
    watchdog_counter++;
    if (watchdog_counter >= WATCHDOG_LIMIT) {
        draw_panic_screen(" KERNEL PANIC: WATCHDOG TIMEOUT ");
        while(1);
    }
}

void watchdog_reset() {
    watchdog_counter = 0;
}