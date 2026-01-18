[bits 32]
extern main
global _start

_start:
    ; Не нужно переназначать ESP здесь, мы уже сделали это в boot.asm
    ; Но если хочешь подстраховаться, используй тот же адрес:
    mov esp, 0x7FFFF
    call main
    jmp $