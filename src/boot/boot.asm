[org 0x7c00]
KERNEL_OFFSET equ 0x1000

jmp start
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

    mov [BOOT_DRIVE], dl

    mov ax, 0x0003
    int 0x10

    mov si, MSG_BOOTING
    call print_string_16

    call enable_a20
    
    mov si, MSG_LOAD_KERNEL
    call print_string_16
    call load_kernel

    mov si, MSG_SWITCH_PM
    call print_string_16
    
    ; --- АНТИ-BOOTLOOP ТЕСТ №1 (Real Mode) ---
    ; Если мы здесь, значит ядро загружено, A20 включен.
    
    call switch_to_pm
    jmp $

[bits 16]
print_string_16:
    mov ah, 0x0e
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

load_kernel:
    mov [RETRY_COUNT], byte 3 ; Попробуем 3 раза, если диск тупит
.retry:
    mov bx, KERNEL_OFFSET
    mov dh, 0
    mov ch, 0
    mov cl, 2           ; Сектор 2
    mov al, KERNEL_SECTORS
    mov ah, 0x02
    mov dl, [BOOT_DRIVE]
    int 0x13
    
    jnc .success        ; Если флаг переноса (CF) = 0, всё ок!
    
    ; Если ошибка — сбрасываем контроллер диска и пробуем еще раз
    xor ax, ax          ; ah = 0 (Reset Disk Drive)
    int 0x13
    
    dec byte [RETRY_COUNT]
    jnz .retry          ; Если еще остались попытки — прыгаем в начало
    
    jmp disk_error      ; Если всё равно не вышло — фатал

.success:
    ret

RETRY_COUNT db 0

disk_error:
    mov si, MSG_DISK_ERROR
    call print_string_16
    jmp $

disk_sectors_error:
    mov si, MSG_SECTORS_ERROR
    call print_string_16
    jmp $

switch_to_pm:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ; ВНИМАНИЕ: Если здесь зависает или ребутит — проблема в GDT
    jmp 0x08:init_pm 

; --- GDT с выравниванием ---
align 4
gdt_start:
    dq 0x0
gdt_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0
gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
init_pm:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov ebp, 0x90000 
    mov esp, ebp

    ; --- АНТИ-BOOTLOOP ТЕСТ №2 (Protected Mode) ---
    ; Пишем 'OK' в угол экрана красным цветом.
    ; Если увидишь OK, значит переход в 32 бита УСПЕШЕН.
    mov word [0xb8000], 0x4f4b ; 'K' (0x4b), фон красный (0x4)
    mov word [0xb8002], 0x4f4f ; 'O' (0x4f), фон красный (0x4)

    ; Если после этого ребут — значит падает само ядро в KERNEL_OFFSET
    call KERNEL_OFFSET
    jmp $

BOOT_DRIVE          db 0
MSG_BOOTING         db "CawOS Booting...", 0x0D, 0x0A, 0
MSG_LOAD_KERNEL     db " Loading Kernel...", 0x0D, 0x0A, 0
MSG_SWITCH_PM       db " Jumping to Protected Mode...", 0
MSG_DISK_ERROR      db "FATAL: Disk error!", 0
MSG_SECTORS_ERROR   db "FATAL: Sector mismatch!", 0

times 510-($-$$) db 0
dw 0xAA55