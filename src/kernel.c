#include <stdint.h>
#include "cpu/interrupts/idt.h"
#include <stdbool.h>
#include "drivers/display/text.h"

#define memory_map 0x8000
#define memory_map_entries memory_map + 4

extern uint8_t ATA_init();

const uint8_t *keyboard_modifier_keys = (uint8_t *)0x90001;

const uint8_t *keycode_register = (uint8_t *)0x90003;
const uint8_t *keycode_buffer = (uint8_t *)0x90500;

const uint16_t *Disk_ATA_INDETIFY = (uint16_t *)0x90600;

extern void main()
{
	print("Welcome to Uncertainty OS!\n\n", 0x0f);

	{ // calculate amount of usable memory
		uint32_t total_memory = 0;
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
		print_size_o(total_memory, 0x0f);
		print(" usable memory\n", 0x0f);
	}
	{ // ATA PIO driver
		print("Initializing ATA PIO driver...\n", 0x0f);
		if (ATA_init())
		{
			print("Failed to initialize ATA PIO driver\n", 0x0f);
			while (1)
				;
		}
		else
		{
			print("ATA PIO driver initilized\n", 0x0f);
		};

		uint32_t Disk_size = *(uint32_t *)(&Disk_ATA_INDETIFY[60]) * 512;
		print("Disk size: ", 0x0f);
		print_size_o(Disk_size, 0x0f);
		echo('\n', 0x0f);
	}

	uint8_t local_keycode_register = *keycode_register;
	while (true)
	{
		while (*keycode_register != local_keycode_register)
		{
			echo(keycode_buffer[local_keycode_register], 0x0f);
			local_keycode_register++;
		}
	}
	return;
}
