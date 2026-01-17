[bits 16]
switch_to_pm:
    cli                     ; 1. Отключаем прерывания (обязательно!)
    lgdt [gdt_descriptor]    ; 2. Загружаем GDT
    
    mov eax, cr0
    or eax, 0x1             ; 3. Включаем Protected Mode бит
    mov cr0, eax

    ; 4. КРИТИЧЕСКИЙ МОМЕНТ: Дальний прыжок (Far Jump)
    ; Он принудительно очищает конвейер команд процессора (CPU Pipeline).
    ; Это гарантирует, что следующая инструкция будет считана уже как 32-битная.
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    ; 5. Теперь мы в 32-битном режиме. 
    ; Сразу настраиваем сегменты данных индексами из GDT.
    mov ax, DATA_SEG        
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 6. Настройка стека подальше от кода (0x90000 - отличный выбор)
    mov ebp, 0x90000        
    mov esp, ebp

    ; 7. Переходим к точке входа в ядро
    call BEGIN_PM