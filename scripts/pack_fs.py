import os
import struct

# Настройки CawFS (должны совпадать с fs.h)
SUPERBLOCK_LBA = 64
ROOT_DIR_LBA = 65
DATA_START_LBA = 71
SECTOR_SIZE = 512

def pack_fs(image_path, bin_dir):
    with open(image_path, 'ab') as img:
        # 1. Добиваем образ нулями до суперблока (LBA 64), если он меньше
        current_size = os.path.getsize(image_path)
        target_padding = SUPERBLOCK_LBA * SECTOR_SIZE
        if current_size < target_padding:
            img.write(b'\x00' * (target_padding - current_size))

        # 2. Подготавливаем файлы
        files = [f for f in os.listdir(bin_dir) if f.endswith('.bin')]
        file_entries = []
        current_lba = DATA_START_LBA

        for f_name in files:
            path = os.path.join(bin_dir, f_name)
            size = os.path.getsize(path)
            
            # Структура cawfs_entry_t: name(32), start_lba(4), size(4), is_exe(1), exists(1)
            entry = struct.pack('32sIIBB', 
                                f_name.replace('.bin', '').encode('ascii'), 
                                current_lba, 
                                size, 
                                1, 1)
            file_entries.append(entry)
            
            # Читаем данные файла для записи позже
            with open(path, 'rb') as f_data:
                content = f_data.read()
                # Добиваем файл до размера сектора
                if len(content) % SECTOR_SIZE != 0:
                    content += b'\x00' * (SECTOR_SIZE - (len(content) % SECTOR_SIZE))
                
                # Записываем данные в "конец" (мы их допишем после таблицы)
                # Для этого сначала соберем всё в памяти
            current_lba += (len(content) // SECTOR_SIZE)

        # 3. Записываем таблицу файлов (LBA 65)
        img.seek(ROOT_DIR_LBA * SECTOR_SIZE)
        for entry in file_entries:
            img.write(entry)
        
        # Заполняем остаток сектора таблицы нулями (макс 16 файлов)
        img.write(b'\x00' * (SECTOR_SIZE * 2 - len(file_entries) * 44))

        # 4. Записываем данные файлов (LBA 71+)
        img.seek(DATA_START_LBA * SECTOR_SIZE)
        for f_name in files:
            with open(os.path.join(bin_dir, f_name), 'rb') as f_data:
                content = f_data.read()
                img.write(content)
                if len(content) % SECTOR_SIZE != 0:
                    img.write(b'\x00' * (SECTOR_SIZE - (len(content) % SECTOR_SIZE)))

pack_fs('os-image.bin', 'build/apps/')