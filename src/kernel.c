#include <stdint.h>
#include "interrupts/idt.h"
#include <stdbool.h>

#define VIDEO_MEMORY 0xb8000

void print(char *string, uint8_t color);
void hexprint(uint8_t digit, uint8_t color);

int Cursor;

extern void main()
{
	Cursor = VIDEO_MEMORY + 80 * 2 * 2;

	char *message = "Hello World!";
	print(message, 0x0f);

	idt_init();

	char buffer[9];
	buffer[8] = 0;

	for (int i = 0; true; i++)
	{
		Cursor = VIDEO_MEMORY + 80 * 2 * 2 + 2 * 20;

		hexprint((i >> 28) & 0x0f, 0x0f);
		hexprint((i >> 24) & 0x0f, 0x0f);
		hexprint((i >> 20) & 0x0f, 0x0f);
		hexprint((i >> 16) & 0x0f, 0x0f);
		hexprint((i >> 12) & 0x0f, 0x0f);
		hexprint((i >> 8) & 0x0f, 0x0f);
		hexprint((i >> 4) & 0x0f, 0x0f);
		hexprint((i) & 0x0f, 0x0f);
	}

	return;
}

void print(char *string, uint8_t color)
{
	while (string[0])
	{
		*(char *)(Cursor) = string[0];
		*(uint8_t *)(Cursor + 1) = color;
		Cursor += 2;
		string++;
	}
}

void hexprint(uint8_t digit, uint8_t color)
{
	if (digit < 10)
	{
		*(char *)(Cursor) = digit + '0';
		*(uint8_t *)(Cursor + 1) = color;
	}
	else
	{
		*(char *)(Cursor) = digit + ('a' - 10);
		*(uint8_t *)(Cursor + 1) = color;
	}
	Cursor += 2;
}