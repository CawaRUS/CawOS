#include "util.h"
#include "io.h"

int strcmp(char* s1, char* s2) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

void strcpy(char* dest, char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int strncmp(char* s1, char* s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        if (s1[i] == '\0') return 0;
    }
    return 0;
}

void memset(void* dest, unsigned char val, int len) {
    unsigned char* temp = (unsigned char*)dest;
    for ( ; len != 0; len--) *temp++ = val;
}


void get_cpu_info(char* vendor) {
    unsigned int ebx, ecx, edx;
    __asm__ __volatile__("cpuid" 
                         : "=b"(ebx), "=c"(ecx), "=d"(edx) 
                         : "a"(0));
    
    ((unsigned int*)vendor)[0] = ebx;
    ((unsigned int*)vendor)[1] = edx;
    ((unsigned int*)vendor)[2] = ecx;
    vendor[12] = '\0';
}

// Функция для чтения из портов CMOS
unsigned short get_total_memory() {
    unsigned short total;
    unsigned char low, high;

    port_byte_out(0x70, 0x30);       // Индекс 0x30: Младший байт памяти
    low = port_byte_in(0x71);
    port_byte_out(0x70, 0x31);       // Индекс 0x31: Старший байт памяти
    high = port_byte_in(0x71);

    total = low | (high << 8);       // Склеиваем байты
    return total / 1024;             // Переводим из КБ в МБ
}


int strlen(char* s) {
    int i = 0;
    while (s[i] != '\0') i++;
    return i;
}

void itoa(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    // Переворачиваем строку
    for (int j = 0, k = i-1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}