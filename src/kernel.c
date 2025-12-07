#include <stdint.h>
#include "interrupts/idt.h"
#include <stdbool.h>

#define VIDEO_MEMORY 0xb8000

void echo(char value, uint8_t color);
void print(char *string, uint8_t color);
void hexprint8(uint8_t value, uint8_t color);
void hexprint(uint8_t digit, uint8_t color);

int Cursor;

uint8_t *keycode_register = (uint8_t *)0x9f000;
uint8_t *keycode_buffer = (uint8_t *)0x9f200;

extern void main()
{

	uint8_t local_keycode_register = *keycode_register;

	Cursor = VIDEO_MEMORY + 80 * 2 * 2;

	print("Welcome to Uncertainty OS!", 0x0f);

	Cursor = VIDEO_MEMORY + 80 * 2 * 4;

	while (true)
	{
		while (*keycode_register != local_keycode_register)
		{
			hexprint8(keycode_buffer[local_keycode_register], 0x0f);
			local_keycode_register++;
		}
	}

	return;
}

void print(char *string, uint8_t color)
{
	while (string[0])
	{
		echo(string[0], color);
		string++;
	}
}

void echo(char value, uint8_t color){

		*(char *)(Cursor) = value;
		*(uint8_t *)(Cursor + 1) = color;
		Cursor += 2;
}

void hexprint8(uint8_t value, uint8_t color)
{
	hexprint(value >> 4, color);
	hexprint(value & 0x0f, color);
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