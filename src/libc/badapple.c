#include "util.h"
#include "screen.h"

// Глобальные указатели для парсера
static char* p_src_start; // Чтобы знать, где начало кода
static char* p_src;
static char* tk;
static int tk_len;
static int tk_type;

// Лексер: разбивает исходный код на C-токены
static int get_next_token(void) {
    tk_len = 0; tk_type = 0;
    char c;
    
    // Если исходный код закончился, начинаем читать его сначала
    if (*p_src == '\0') p_src = p_src_start;

    while ((c = *p_src)) {
        tk = p_src++;
        
        // Пропускаем пробелы и переносы
        if (c == ' ' || c == '\r' || c == '\n' || c == '\t') continue;
        
        // Строковые литералы
        if (c == '"' || c == '\'') {
            tk_type = 1; tk_len = 1;
            while (*p_src != '\0' && *p_src != c) {
                if (*p_src == '\\') { tk_len++; p_src++; }
                tk_len++; p_src++;
            }
            tk_len++; p_src++; break;
        } 
        // Идентификаторы и ключевые слова
        else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
            tk_type = 2; tk_len = 1;
            while ((*p_src >= 'a' && *p_src <= 'z') || (*p_src >= 'A' && *p_src <= 'Z') ||
                   (*p_src >= '0' && *p_src <= '9') || *p_src == '_') { p_src++; tk_len++; }
            break;
        } 
        // Числа
        else if (c >= '0' && c <= '9') {
            tk_type = 3; tk_len = 1;
            while ((*p_src >= '0' && *p_src <= '9')) { p_src++; tk_len++; }
            break;
        } 
        // Символы пунктуации
        else { tk_len = 1; break; }
    }
    return tk_len;
}

// Отрисовка кадра: маска определяет, где печатать код, а где пробел
void play_bad_apple_frame(char* source_code, char* mask) {
    p_src_start = source_code;
    p_src = source_code;
    char* curr = mask;
    int cur_row = 0, cur_col = 0;
    int p_is_char = 0;
    int width_acc = 0;

    while (*curr) {
        // Если встретили пробел или перенос строки в маске (черная область)
        if (*curr == ' ' || *curr == '\n') {
            if (p_is_char == 1) {
                // Отрисовываем накопленную "белую" область токенами кода
                int printed = 0;
                while (printed < width_acc) {
                    if (!get_next_token()) break;
                    
                    char tmp[65];
                    int copy_len = (tk_len > 64) ? 64 : tk_len;
                    for(int i = 0; i < copy_len; i++) tmp[i] = tk[i];
                    tmp[copy_len] = '\0';

                    print_at_color(tmp, cur_row, cur_col, 0x0F);
                    cur_col += copy_len;
                    printed += copy_len;
                }
                width_acc = 0;
            }
            
            p_is_char = 0;
            if (*curr == '\n') {
                cur_row++; 
                cur_col = 0;
                if (cur_row >= 25) break; // Защита от выхода за экран
            } else {
                // Важно: затираем старый символ пробелом, чтобы не было "шлейфа"
                print_at(" ", cur_row, cur_col);
                cur_col++;
                if (cur_col >= 80) { cur_col = 0; cur_row++; }
            }
        } 
        // Если встретили звездочку (белая область видео)
        else {
            p_is_char = 1;
            width_acc++;
        }
        curr++;
    }
}