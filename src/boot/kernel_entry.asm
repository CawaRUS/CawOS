[bits 32]
extern main
global _start

_start:
    mov esp, 0x7FFFF ; Ставим стек
    call main        ; Уходим в ядро
    jmp $