#include <stdint.h>
#include "cpu/interrupts/idt.h"
#include <stdbool.h>
#include "drivers/display/text.h"

#define memory_map 0x8000
#define memory_map_entries memory_map + 4

uint8_t *keyboard_modifier_keys = (uint8_t *)0x90001;

uint8_t *keycode_register = (uint8_t *)0x90003;
uint8_t *keycode_buffer = (uint8_t *)0x90500;

extern void main()
{
	uint32_t Cursor = VIDEO_MEMORY;
	print("Welcome to Uncertainty OS!\n\n", 0x0f, &Cursor);

	print("| Base adress    | Lentgh         | Type   |\n", 0x0f, &Cursor);

	for (uint32_t entry_id = 0; entry_id < *(uint32_t *)memory_map; entry_id++)
	{
		echo(' ', 0x0f, &Cursor);
		for (int8_t i = 7; i >= 0; i--)
		{
			hexprint8(*(uint8_t *)(memory_map + entry_id * 24 + i + 4), 0x0f, &Cursor);
		}

		echo(' ', 0x0f, &Cursor);
		for (int8_t i = 15; i >= 8; i--)
		{
			hexprint8(*(uint8_t *)(memory_map + entry_id * 24 + i + 4), 0x0f, &Cursor);
		}

		echo(' ', 0x0f, &Cursor);
		for (int8_t i = 19; i >= 16; i--)
		{
			hexprint8(*(uint8_t *)(memory_map + entry_id * 24 + i + 4), 0x0f, &Cursor);
		}
		echo(10, 0x0f, &Cursor);
	}

	uint32_t total_memory = 0;

	// calculate amount of usable memory
	bool loop = true;
	for (uint32_t entry_id = 0; (entry_id < (*(uint32_t *)memory_map)) && loop; entry_id++)
	{
		if (*(uint32_t *)(memory_map_entries + entry_id * 24 + 4) == 0)
		{
			if (*(uint32_t *)(memory_map_entries + entry_id * 24 + 16) == 1)
			{
				total_memory += *(uint32_t *)(memory_map_entries + entry_id * 24 + 8);
			}
		}
		else
		{
			loop = false;
		}
	}
	// print it
	if (total_memory < (uint32_t)(1024 * 10000))
	{
		print_d10(total_memory / 1024, 0x0f, &Cursor);
		print(" KiB", 0x0f, &Cursor);
	}
	else
	{
		print_d10(total_memory / (1024 * 1024), 0x0f, &Cursor);
		print(" MiB", 0x0f, &Cursor);
	}

	print(" of free memory\n\n", 0x0f, &Cursor);

	uint8_t local_keycode_register = *keycode_register;
	while (true)
	{
		while (*keycode_register != local_keycode_register)
		{
			echo(keycode_buffer[local_keycode_register], 0x0f, &Cursor);
			local_keycode_register++;
		}
	}
	return;
}
