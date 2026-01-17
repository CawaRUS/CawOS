@echo off
setlocal enabledelayedexpansion
echo Building CawOS...

:: Создаем папку build, если её нет
if not exist build mkdir build

:: 1. Ассемблируем загрузчик и вход в ядро
echo [ASM] Assembling boot and entry...
nasm src/boot/boot.asm -f bin -o build/boot.bin
nasm src/boot/kernel_entry.asm -f elf32 -o build/kernel_entry.o

:: 2. Компилируем Си-файлы (добавлен -Iinclude для поиска заголовков)
echo [C] Compiling kernel modules...
i686-elf-gcc -ffreestanding -fno-pie -fno-stack-protector -m32 -Iinclude -c src/kernel/kernel.c -o build/kernel.o
i686-elf-gcc -ffreestanding -fno-pie -fno-stack-protector -m32 -Iinclude -c src/drivers/io.c -o build/io.o
i686-elf-gcc -ffreestanding -fno-pie -fno-stack-protector -m32 -Iinclude -c src/drivers/screen.c -o build/screen.o
i686-elf-gcc -ffreestanding -fno-pie -fno-stack-protector -m32 -Iinclude -c src/fs/fs.c -o build/fs.o
i686-elf-gcc -ffreestanding -fno-pie -fno-stack-protector -m32 -Iinclude -c src/libc/util.c -o build/util.o

:: 3. Линкуем всё в один ELF (используем linker.ld из папки scripts)
echo [LD] Linking kernel.elf...
i686-elf-ld -m elf_i386 -T scripts/linker.ld -nostdlib ^
build/kernel_entry.o ^
build/kernel.o ^
build/io.o ^
build/screen.o ^
build/fs.o ^
build/util.o ^
-o build/kernel.elf

:: 4. Извлекаем чистый бинарный код ядра
i686-elf-objcopy -O binary build/kernel.elf build/kernel.bin

:: 5. Собираем финальный образ
echo [BIN] Creating os-image.bin...
:: Создаем Padding (20КБ)
fsutil file createnew build/pad.bin 20480 > nul
copy /b build\boot.bin + build\kernel.bin + build\pad.bin os-image.bin > nul

echo Done! CawOS image is ready.
pause