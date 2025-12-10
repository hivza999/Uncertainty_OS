#include <stdint.h>

#define VIDEO_MEMORY 0xb8000
#define Scren_Width 80
#define Scren_Height 25

void print_d10(uint32_t value, uint8_t color, uint32_t *p_cursor);
void echo(char value, uint8_t color, uint32_t *p_cursor);
void print(char *string, uint8_t color, uint32_t *p_cursor);
void hexprint8(uint8_t value, uint8_t color, uint32_t *p_cursor);
void hexprint(uint8_t digit, uint8_t color, uint32_t *p_cursor);
