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

    ; 2. Включаем A20
    call enable_a20
    mov si, MSG_A20_OK
    call print_string_16

    ; 3. Загрузка ядра (53 сектора, как ты настроил)
    mov si, MSG_LOAD_KERNEL
    call print_string_16
    call load_kernel

    ; --- МЕНЮ ВЫБОРА РЕЖИМА ---
    mov si, MSG_CHOOSE_MODE
    call print_string_16

.wait_key:
    mov ah, 0x00
    int 0x16            ; Ждем нажатия клавиши

    cmp al, '1'
    je .set_text_mode
    cmp al, '2'
    je .set_graph_mode
    jmp .wait_key       ; Игнорируем другие клавиши

.set_text_mode:
    mov ax, 0x0003      ; Стандартный текстовый режим 80x25
    int 0x10
    jmp .finish_boot

.set_graph_mode:
    mov ax, 0x0013      ; Графический режим 13h (320x200 256c)
    int 0x10
    jmp .finish_boot

.finish_boot:
    ; 4. Переход в Protected Mode
    mov si, MSG_SWITCH_PM
    call print_string_16
    
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
    mov cl, 2           ; Сектор 2 (сразу после бутлоадера)
    mov al, 53          ; Читаем сектора
    mov ah, 0x02
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc disk_error
    
    cmp al, 53
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
    
    ; Стек подальше от ядра
    mov ebp, 0x90000 
    mov esp, ebp

    call KERNEL_OFFSET
    jmp $

; --- Данные ---
BOOT_DRIVE          db 0
MSG_BOOTING         db "CawOS Booting...", 0x0D, 0x0A, 0
MSG_A20_OK          db " A20 Line: OK", 0x0D, 0x0A, 0
MSG_LOAD_KERNEL     db " Kernel Loading...", 0x0D, 0x0A, 0
MSG_CHOOSE_MODE     db 0x0D, 0x0A, "Select Boot Mode:", 0x0D, 0x0A, " 1. Text Mode (CLI)", 0x0D, 0x0A, " 2. Graphics Mode (GUI)", 0x0D, 0x0A, "Choice: ", 0
MSG_SWITCH_PM       db 0x0D, 0x0A, " Jumping to Protected Mode...", 0
MSG_DISK_ERROR      db "FATAL: Disk error!", 0
MSG_SECTORS_ERROR   db "FATAL: Sector mismatch!", 0

times 510-($-$$) db 0
dw 0xAA55