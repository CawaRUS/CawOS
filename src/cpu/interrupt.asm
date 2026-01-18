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
    pusha
    mov ax, ds
    push eax       ; Сохраняем DS
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax

    push esp       ; Передаем ВЕСЬ стек как структуру registers
    call isr_handler
    add esp, 4     ; Убираем только пуш esp
    
    pop eax        ; Восстанавливаем DS
    mov ds, ax
    mov es, ax
    
    popa
    add esp, 8     ; Убираем int_no и err_code
    iret