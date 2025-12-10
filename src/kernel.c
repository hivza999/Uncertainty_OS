#include <stdint.h>
#include "cpu/interrupts/idt.h"
#include <stdbool.h>

#define memory_map 0x8000
#define VIDEO_MEMORY 0xb8000

void echo(char value, uint8_t color, uint32_t *p_cursor);
void print(char *string, uint8_t color, uint32_t *p_cursor);
void hexprint8(uint8_t value, uint8_t color, uint32_t *p_cursor);
void hexprint(uint8_t digit, uint8_t color, uint32_t *p_cursor);

uint8_t *keyboard_modifier_keys = (uint8_t *)0x90001;

uint8_t *keycode_register = (uint8_t *)0x90003;
uint8_t *keycode_buffer = (uint8_t *)0x90500;

extern void main()
{
	uint8_t local_keycode_register = *keycode_register;

	uint32_t Cursor = VIDEO_MEMORY;
	print("Welcome to Uncertainty OS!", 0x0f, &Cursor);

	Cursor = VIDEO_MEMORY + 160 * 2;
	print("Memory map", 0x0f, &Cursor);

	Cursor = VIDEO_MEMORY + 160 * 3;
	print("| Base adress    | Lentgh         | Type   |", 0x0f, &Cursor);

	for (uint32_t entry_id = 0; entry_id < *(uint32_t *)memory_map; entry_id++)
	{
		Cursor = VIDEO_MEMORY + 80 * 2 * (entry_id + 4) + 2;
		for (int8_t i = 7; i >= 0; i--)
		{
			hexprint8(*(uint8_t *)(memory_map + entry_id * 24 + i + 4), 0x0f, &Cursor);
		}

		Cursor += 2;
		for (int8_t i = 15; i >= 8; i--)
		{
			hexprint8(*(uint8_t *)(memory_map + entry_id * 24 + i + 4), 0x0f, &Cursor);
		}

		Cursor += 2;
		for (int8_t i = 19; i >= 16; i--)
		{
			hexprint8(*(uint8_t *)(memory_map + entry_id * 24 + i + 4), 0x0f, &Cursor);
		}
	}

	Cursor = VIDEO_MEMORY + 80 * 2 * (*(uint32_t *)memory_map + 5);

	while (true)
	{
		uint32_t tmp32 = VIDEO_MEMORY + 80;
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