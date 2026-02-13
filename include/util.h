#ifndef UTIL_H
#define UTIL_H

// Строковые функции (теперь с const, чтобы не было ворнингов в командах)
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, int n);
int strlen(const char* s);

void strcpy(char* dest, const char* src); // src тоже const, мы его только читаем
void memset(void* dest, unsigned char val, int len);

// Системные функции
void get_cpu_info(char* buffer);
unsigned short get_total_memory();
void itoa(int n, char str[]);

#endif