#ifndef VGA_H
#define VGA_H

void vga_enter_mode13h();
void draw_pixel(int x, int y, unsigned char color);


void gui_clear_screen(unsigned char color);
void gui_draw_rect(int x, int y, int width, int height, unsigned char color);
void gui_draw_window(int x, int y, int w, int h, char* title);

#endif