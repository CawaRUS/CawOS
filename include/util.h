#ifndef UTIL_H
#define UTIL_H

int strcmp(char* s1, char* s2);
void strcpy(char* dest, char* src);
void memset(void* dest, unsigned char val, int len);
int strncmp(char* s1, char* s2, int n);

void get_cpu_info(char* buffer);

extern unsigned short get_total_memory();
extern void itoa(int n, char str[]);
extern int strlen(char* s);

#endif