ENTRY(_start)

SECTIONS
{
    . = 0x1000;

    .text   : { *(.text) }
    .rodata : { *(.rodata*) }
    .data   : { *(.data) }
    .bss    : { *(.bss) *(COMMON) }

    .cmd : ALIGN(4) {
        __start_cmd = .;    /* Создаем метку начала списка команд */
        KEEP(*(.cmd))       /* Сваливаем сюда все REGISTER_COMMAND из всех файлов */
        __stop_cmd = .;     /* Создаем метку конца */
    }

    /DISCARD/ : { *(.comment) *(.eh_frame) }
}