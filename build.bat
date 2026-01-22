@echo off
setlocal enabledelayedexpansion
color 07
echo =================================
echo         Building CawOS
echo =================================

:: Проверка наличия компилятора
i686-elf-gcc --version >nul 2>&1
if %errorlevel% neq 0 (
    color 0C
    echo [ERROR] i686-elf-gcc not found in PATH!
    pause
    exit /b
)

:: Создаем структуру
if not exist build mkdir build

:: Очистка
echo [CLEAN] Removing old binaries...
del /f /q build\*.bin build\*.o build\*.elf os-image.bin 2>nul

:: 1. Ассемблер (статичные файлы)
echo [ASM] Assembling boot and low-level code...
nasm src/boot/boot.asm -f bin -o build/boot.bin || goto :error
nasm src/boot/kernel_entry.asm -f elf32 -o build/kernel_entry.o || goto :error
nasm src/cpu/interrupt.asm -f elf32 -o build/interrupt.o || goto :error

:: 2. Компиляция всех .c файлов автоматически
echo [C] Compiling all kernel modules...
set C_FLAGS=-ffreestanding -fno-pie -fno-stack-protector -m32 -Iinclude -Wall -O0 -c
set "OBJ_FILES="

:: Рекурсивный поиск всех .c файлов в папке src
for /r src %%f in (*.c) do (
    echo   Compiling %%~nxf...
    i686-elf-gcc %C_FLAGS% "%%f" -o "build\%%~nf.o" || goto :error
    :: Добавляем объектный файл в список для линковки
    set "OBJ_FILES=!OBJ_FILES! build\%%~nf.o"
)

:: 3. Линковка
echo [LD] Linking kernel.elf...
:: Линкуем ассемблерные заглушки + все найденные объектники
i686-elf-ld -m elf_i386 -T scripts/linker.ld -nostdlib ^
build/kernel_entry.o build/interrupt.o !OBJ_FILES! ^
-o build/kernel.elf || goto :error

:: 4. Извлечение бинарника
echo [OBJ] Extracting kernel.bin...
i686-elf-objcopy -O binary build/kernel.elf build/kernel.bin

:: 5. Сборка образа с ПАДДИНГОМ
echo [BIN] Finalizing os-image.bin...
if exist build\pad.bin del /f /q build\pad.bin
fsutil file createnew build\pad.bin 32768 > nul

copy /b build\boot.bin + build\kernel.bin + build\pad.bin os-image.bin > nul

echo.
color 0A
echo [SUCCESS] CawOS is ready to fly!
for %%I in (os-image.bin) do echo Final Image Size: %%~zI bytes
echo.
pause
exit /b

:error
color 0C
echo.
echo [FATAL ERROR] Build failed! Check the logs above.
pause
exit /b