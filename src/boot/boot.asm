[org 0x7c00]
KERNEL_OFFSET equ 0x1000

; --- BIOS Parameter Block (BPB) ---
; Мы делаем жесткий отступ, чтобы BIOS не перезаписал наш код
jmp short start
nop
times 33 db 0 

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti

    ; На всякий случай проверяем DL. Если он 0, ставим 0x80 (первый диск)
    test dl, dl
    jnz .save_dl
    mov dl, 0x80
.save_dl:
    mov [BOOT_DRIVE], dl

    ; Точка жизни
    mov ah, 0x0e
    mov al, '.'
    int 0x10

    call enable_a20
    call load_kernel
    call switch_to_pm
    jmp $

[bits 16]
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

load_kernel:
    mov bx, KERNEL_OFFSET
    mov dh, 0
    mov ch, 0
    mov cl, 2       ; Сектор 2
    mov al, 20      ; Читаем 20 секторов (с запасом)
    mov ah, 0x02
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc disk_error
    ret

disk_error:
    mov ah, 0x0e
    mov al, 'E'
    int 0x10
    jmp $

switch_to_pm:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ; Явное указание сегмента 0x08 для прыжка
    jmp 0x08:init_pm 

; --- GDT (Компактный и строгий) ---
gdt_start: dq 0
gdt_code: dw 0xffff, 0, 0x9a00, 0x00cf
gdt_data: dw 0xffff, 0, 0x9200, 0x00cf
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
init_pm:
    mov ax, 0x10    ; DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Переносим стек чуть ниже, чтобы он не залез на ядро
    mov ebp, 0x7FFFF 
    mov esp, ebp

    call KERNEL_OFFSET
    jmp $

BOOT_DRIVE db 0
times 510-($-$$) db 0
dw 0xAA55