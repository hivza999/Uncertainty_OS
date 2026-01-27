#include <stdint.h>
#include "cpu/interrupts/idt.h"
#include <stdbool.h>
#include "drivers/display/text.h"

#define memory_map 0x8000
#define memory_map_entries memory_map + 4

#define Device_ATA 1
#define Device_PATAPI 2
#define Device_SATAPI 3
#define Device_PATA 4
#define Device_SATA 5
#define Device_Unknowed 255

extern uint32_t ATA_init();
extern int ATA_PIO_read();

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
		print_size_B(total_memory, 0x0f);
		print(" usable memory\n", 0x0f);
	}
	{ // ATA PIO driver
		print("Initializing ATA PIO driver...\n", 0x0f);
		uint32_t Detected_drives = ATA_init();
		print("ATA PIO driver initilized\n", 0x0f);

		for (uint8_t i = 0; i < 4; i++)
		{
			if (!((uint8_t *)&Detected_drives)[i])
			{
				continue;
			}
			print("Storage device ", 0x0f);
			echo(i + '0', 0x0f);
			print("\n  Device type: ", 0x0f);

			switch (((uint8_t *)&Detected_drives)[i])
			{
			case Device_ATA:
				print("ATA\n  LBA48 ", 0x0f);

				uint64_t Device_size;
				if (*(uint16_t *)&(Disk_ATA_INDETIFY[256 * i + 83]) & (1 << 10))
				{ // get if LBA48 is supported
					Device_size = *(uint64_t *)&(Disk_ATA_INDETIFY[256 * i + 100]) * 512;
				}
				else
				{
					Device_size = *(uint32_t *)&(Disk_ATA_INDETIFY[256 * i + 60]) * 512;
					print("un", 0x0f);
				}
				print("supported\n  Device size: ", 0x0f);
				print_size_B(Device_size, 0x0f); // print size of the disk
				echo('\n', 0x0f);
				break;

			case Device_PATAPI:
				print("PATAPI\n", 0x0f);
				break;

			case Device_SATAPI:
				print("SATAPI\n", 0x0f);
				break;

			case Device_PATA:
				print("PATA\n", 0x0f);
				break;

			case Device_SATA:
				print("SATA\n", 0x0f);
				break;

			case Device_Unknowed:
				print("Unknow\n", 0x0f);
				break;
			}
		}
	}

	ATA_PIO_read();
	echo(0x1b, 0x0f);
	for (uint32_t i = 0; i < 512; i++)
	{
		hexprint8(*((uint8_t *)i + 0x8fe00), 0x0f);
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
