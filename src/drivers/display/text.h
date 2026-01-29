#pragma once
#include <stdint.h>

#define VIDEO_MEMORY 0xb8000
#define Scren_Width 80
#define Scren_Height 25

void print_size_B(uint64_t value);
void print_d10(uint32_t value);
void echo(char value);
void print(char *string);
void hexprint64(uint64_t value);
void hexprint32(uint32_t value);
void hexprint16(uint16_t value);
void hexprint8(uint8_t value);
void hexprint(uint8_t digit);
void screen_scroll(uint8_t lines);
void cursor_color(uint8_t color);
