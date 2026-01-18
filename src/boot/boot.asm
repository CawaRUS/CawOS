[org 0x7c00]
KERNEL_OFFSET equ 0x1000

; --- BIOS Parameter Block (BPB) ---
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

    test dl, dl
    jnz .save_dl
    mov dl, 0x80
.save_dl:
    mov [BOOT_DRIVE], dl

    ; 1. Приветствие
    mov si, MSG_BOOTING
    call print_string_16

    ; 2. Включаем A20 и отчитываемся
    call enable_a20
    mov si, MSG_A20_OK
    call print_string_16

    ; 3. Загрузка ядра
    mov si, MSG_LOAD_KERNEL
    call print_string_16
    call load_kernel

    ; 4. Переход в PM
    mov si, MSG_SWITCH_PM
    call print_string_16
    
    ; Небольшая задержка, чтобы успеть прочитать (по желанию)
    ; mov cx, 0xFFFF
    ; .delay: loop .delay

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
    mov bx, KERNEL_OFFSET
    mov dh, 0
    mov ch, 0
    mov cl, 2       ; Сектор 2
    mov al, 52      ; Считываем 52 сектора
    mov ah, 0x02
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc disk_error
    
    ; Проверка: BIOS возвращает в AL количество реально считанных секторов
    cmp al, 52
    jne disk_sectors_error
    ret

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
    jmp 0x08:init_pm 

; --- GDT ---
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
    
    mov ebp, 0x7FFFF 
    mov esp, ebp

    call KERNEL_OFFSET
    jmp $

; --- Данные (Строки) ---
BOOT_DRIVE          db 0
MSG_BOOTING         db "Booting CawOS...", 0x0D, 0x0A, 0
MSG_A20_OK          db " -> A20 line enabled", 0x0D, 0x0A, 0
MSG_LOAD_KERNEL     db " -> Loading kernel (52 sectors)...", 0x0D, 0x0A, 0
MSG_SWITCH_PM       db " -> Jumping to Protected Mode...", 0x0D, 0x0A, 0
MSG_DISK_ERROR      db "FATAL: Disk read failed!", 0x0D, 0x0A, 0
MSG_SECTORS_ERROR   db "FATAL: Incorrect sector count read!", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55