#include <stdint.h>

#define VIDEO_MEMORY 0xb8000
#define Scren_Width 80
#define Scren_Height 25

void print_size_B(uint64_t value, uint8_t color);
void print_d10(uint32_t value, uint8_t color);
void echo(char value, uint8_t color);
void print(char *string, uint8_t color);
void hexprint8(uint8_t value, uint8_t color);
void hexprint16(uint16_t value, uint8_t color);
void hexprint32(uint32_t value, uint8_t color);
void hexprint64(uint64_t value, uint8_t color);
void hexprint(uint8_t digit, uint8_t color);
