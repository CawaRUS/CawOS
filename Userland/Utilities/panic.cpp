#include <AK/String.h>
#include <AK/StringView.h>
#include <AK/Vector.h>
#include <LibCore/ArgsParser.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <syscall.h>

// --- Функции, вызывающие сбой ---

static void do_divide_zero()
{
    volatile int x = 1;
    volatile int y = 0;
    volatile int z = x / y;
    (void)z;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"

[[gnu::noinline]]
static void recurse_forever(int x)
{
    volatile int dummy = x;
    recurse_forever(dummy + 1);
}

#pragma GCC diagnostic pop

static void do_segfault()
{
    printf("Attempting to write to V0x0000000000000000...\n");
    volatile int* null_ptr = nullptr;
    *null_ptr = 42;
}

// НОВОЕ: Вызывает гарантированную нелегальную инструкцию (UD2)
static void do_illegal_instruction()
{
    printf("Executing illegal instruction (SIGILL)... \n");
    // Инструкция UD2 (Undefined Instruction) гарантированно вызывает исключение (Interrupt 6)
    asm volatile("ud2");
}

// --- Функция подтверждения с тройным предупреждением ---

static void require_confirmation(StringView action)
{
    printf("\n\n!!! ТРОЙНОЕ ПРЕДУПРЕЖДЕНИЕ !!!\n");
    printf("!!! Вы собираетесь выполнить команду 'panic %s' !!!\n", action.characters_without_null_termination());
    printf("!!! ЭТО ВЫЗОВЕТ АВАРИЙНОЕ ЗАВЕРШЕНИЕ ПРОГРАММЫ ИЛИ СБОЙ СИСТЕМЫ (KERNEL PANIC) !!!\n");
    printf("!!! НЕ ЗАПУСКАЙТЕ В КРИТИЧЕСКИХ ПРОЦЕССАХ, ЕСЛИ НЕ ХОТИТЕ ПАНИКИ ЯДРА!!!\n");

    for (int i = 1; i <= 3; ++i) {
        printf("\nПРЕДУПРЕЖДЕНИЕ %d/3: Нажмите ENTER, чтобы продолжить (или Ctrl+C для отмены)...\n", i);
        
        int c = getchar();
        if (c == EOF) {
            printf("\nДействие отменено (EOF).\n");
            exit(1); 
        }

        if (c != '\n') {
            int discard_char;
            while ((discard_char = getchar()) != '\n' && discard_char != EOF);
        }
    }
}


int main(int argc, char** argv)
{
    Vector<StringView> args;
    for (int i = 1; i < argc; i++) {
        if (argv[i]) {
            args.append(StringView(argv[i], strlen(argv[i])));
        }
    }

    if (args.is_empty()) {
        printf("Usage: panic <type>\n");
        printf("Types:\n");
        printf("  fpe         - division by zero (Userland fault)\n");
        printf("  stack       - stack overflow (Userland fault)\n");
        printf("  abort       - abort() call (Userland signal)\n");
        printf("  segfault    - write to NULL pointer (Hard Userland fault)\n");
        printf("  hardcrash   - execute illegal instruction (SIGILL)\n");
        printf("  kpanic      - execute illegal instruction, for KERNEL PANIC context\n"); // НОВОЕ/ИЗМЕНЕННОЕ!
        printf("  unknownsys  - call non-existent syscall 1337\n");
        return 0;
    }

    auto action = args[0];
    
    // --- Пользовательские ошибки (Userland Faults) ---
    if (action == "fpe"sv) {
        require_confirmation(action);
        printf("Triggering FPE (SIGFPE)...\n");
        do_divide_zero();
    }

    if (action == "stack"sv) {
        require_confirmation(action);
        printf("Triggering stack overflow (SIGSEGV/SIGABRT)...\n");
        recurse_forever(1);
    }

    if (action == "abort"sv) {
        require_confirmation(action);
        printf("Triggering abort() (SIGABRT)...\n");
        abort();
    }
    
    if (action == "segfault"sv) {
        require_confirmation(action);
        printf("Triggering Segment Fault (SIGSEGV)...\n");
        do_segfault();
    }

    if (action == "hardcrash"sv) {
        require_confirmation(action);
        printf("Triggering Hard Crash via illegal instruction (SIGILL)...\n");
        do_illegal_instruction();
    }

    // --- Прямой Kernel Panic (Имитация) ---
    if (action == "kpanic"sv) {
        require_confirmation(action);
        printf("Triggering KERNEL PANIC attempt via Illegal Instruction.\n");
        printf("*** ПАНИКА ЯДРА ПРОИЗОЙДЕТ ТОЛЬКО, если 'panic' ЗАПУЩЕН ВМЕСТО КРИТИЧЕСКОГО ПРОЦЕССА (SystemServer, Init). ***\n");
        do_illegal_instruction();
    }
    
    // Вызов неизвестного системного вызова
    if (action == "unknownsys"sv) {
        require_confirmation(action);
        printf("Triggering UNKNOWN SYSCALL (1337). Process will exit with 'Unknown syscall' error.\n");
        syscall(1337); 
    }

    printf("Unknown action '%s'!\n", action.characters_without_null_termination());
    return 1;
}