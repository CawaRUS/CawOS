[bits 32]
extern main
global _start

_start:
    mov esp, 0x90000
    and esp, 0xFFFFFFF0 ; Выравнивание стека по 16 байт (GCC это обожает)
    sub esp, 12         ; Padding для вызова
    call main
    jmp $