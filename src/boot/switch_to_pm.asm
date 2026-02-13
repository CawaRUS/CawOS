[bits 16]
switch_to_pm:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    ; Дальний прыжок обязателен для переключения сегмента кода на 32-битный
    jmp 0x08:init_pm_32 

[bits 32]
init_pm_32:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov ebp, 0x90000 
    mov esp, ebp

    call KERNEL_OFFSET
    jmp $