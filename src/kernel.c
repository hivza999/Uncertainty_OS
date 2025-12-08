#include <stdint.h>
#include "interrupts/idt.h"
#include <stdbool.h>

#define VIDEO_MEMORY 0xb8000

void echo(char value, uint8_t color, uint32_t *p_cursor);
void print(char *string, uint8_t color, uint32_t *p_cursor);
void hexprint8(uint8_t value, uint8_t color, uint32_t *p_cursor);
void hexprint(uint8_t digit, uint8_t color, uint32_t *p_cursor);

uint8_t *keyboard_modifier_keys = (uint8_t *)0x9f001;

uint8_t *keycode_register = (uint8_t *)0x9f003;
uint8_t *keycode_buffer = (uint8_t *)0x9f500;

extern void main()
{

	uint8_t local_keycode_register = *keycode_register;

	uint32_t Cursor = VIDEO_MEMORY + 80 * 2 * 2;

	print("Welcome to Uncertainty OS!", 0x0f, &Cursor);

	Cursor = VIDEO_MEMORY + 80 * 2 * 4;

	while (true)
	{
		uint32_t tmp32 = (VIDEO_MEMORY + 80 * 2 * 2 + 80);

		hexprint8(*keyboard_modifier_keys, 0x0f, &tmp32);

		while (*keycode_register != local_keycode_register)
		{
			echo(keycode_buffer[local_keycode_register], 0x0f, &Cursor);
			local_keycode_register++;
		}
	}

	return;
}

void print(char *string, uint8_t color, uint32_t *p_cursor)
{
	while (string[0])
	{
		echo(string[0], color, p_cursor);
		string++;
	}
}

void echo(char value, uint8_t color, uint32_t *p_cursor)
{

	*(char *)(*p_cursor) = value;
	*(uint8_t *)(*p_cursor + 1) = color;
	*p_cursor += 2;
}

void hexprint8(uint8_t value, uint8_t color, uint32_t *p_cursor)
{
	hexprint(value >> 4, color, p_cursor);
	hexprint(value & 0x0f, color, p_cursor);
}

void hexprint(uint8_t digit, uint8_t color, uint32_t *p_cursor)
{
	if (digit < 10)
	{
		*(char *)(*p_cursor) = digit + '0';
		*(uint8_t *)(*p_cursor + 1) = color;
	}
	else
	{
		*(char *)(*p_cursor) = digit + ('a' - 10);
		*(uint8_t *)(*p_cursor + 1) = color;
	}
	*p_cursor += 2;
}