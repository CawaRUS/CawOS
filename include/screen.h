#ifndef SCREEN_H
#define SCREEN_H

extern unsigned char current_color;

void clear_screen();
void print_at_color(char* message, int row, int col, unsigned char color);
void print_at(char* message, int row, int col);
void update_cursor(int row, int col);
void draw_logo();
void play_sound(unsigned int nFrequence);
void nosound();
void beep();

#endif