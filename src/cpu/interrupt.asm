[bits 32]
extern isr_handler
global isr0
global idt_load

global isr32

idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

isr0:
    push byte 0    ; Код ошибки (заглушка)
    push byte 0    ; Номер прерывания
    jmp isr_common_stub

isr32:
    push byte 0    ; Заглушка для кода ошибки
    push byte 32   ; Номер прерывания (IRQ0)
    jmp isr_common_stub

isr_common_stub:
    pusha               ; Пушит eax, ecx, edx, ebx, esp, ebp, esi, edi
    
    mov ax, ds          ; Сохраняем текущий DS
    push eax            
    
    mov ax, 0x10        ; Загружаем сегмент данных ядра
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; Передаем указатель на структуру registers r
    call isr_handler
    
    add esp, 4          ; Очищаем указатель (esp)
    
    pop eax             ; Восстанавливаем оригинальный DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                ; Восстанавливаем все регистры
    add esp, 8          ; Очищаем номера прерываний (int_no и err_code)
    iret                ; ВОЗВРАЩАЕМСЯ В ОС