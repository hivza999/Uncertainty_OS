#include <stdint.h>
#include "cpu/interrupts/idt.h"
#include <stdbool.h>
#include "drivers/display/text.h"
#include "drivers/storage/ata.h"
#include "drivers/partition_table/mbr.h"
#include "drivers/filesystem/fat.h"

#define memory_map 0x8000
#define memory_map_entries memory_map + 4

#define Device_ATA 1
#define Device_PATAPI 2
#define Device_SATAPI 3
#define Device_PATA 4
#define Device_SATA 5
#define Device_Unknowed 255

const uint8_t *keyboard_modifier_keys = (uint8_t *)0x90001;

const uint8_t *keycode_register = (uint8_t *)0x90003;
const uint8_t *keycode_buffer = (uint8_t *)0x90500;

const uint16_t *Disk_ATA_INDETIFY = (uint16_t *)0x90600;

void ls(char *path, FAT_filesystem_t *FAT_filesystem);

extern void main()
{
	print("Welcome to Uncertainty OS!\n\n");

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
		print_size_B(total_memory);
		print(" usable memory\n");
	}
	{ // ATA PIO driver
		print("Initializing ATA PIO driver...\n");
		uint32_t Detected_drives = ATA_init();
		print("ATA PIO driver initilized\n");

		for (uint8_t i = 0; i < 4; i++)
		{
			if (!((uint8_t *)&Detected_drives)[i])
			{
				continue;
			}
			print("Storage device ");
			echo(i + '0');
			print("\n  Device type: ");

			switch (((uint8_t *)&Detected_drives)[i])
			{
			case Device_ATA:
				print("ATA\n  LBA48 ");

				uint64_t Device_size;
				if (*(uint16_t *)&(Disk_ATA_INDETIFY[256 * i + 83]) & (1 << 10))
				{ // get if LBA48 is supported
					Device_size = *(uint64_t *)&(Disk_ATA_INDETIFY[256 * i + 100]) * 512;
				}
				else
				{
					Device_size = *(uint32_t *)&(Disk_ATA_INDETIFY[256 * i + 60]) * 512;
					print("un");
				}
				print("supported\n  Device size: ");
				print_size_B(Device_size); // print size of the disk
				echo('\n');
				break;

			case Device_PATAPI:
				print("PATAPI\n");
				break;

			case Device_SATAPI:
				print("SATAPI\n");
				break;

			case Device_PATA:
				print("PATA\n");
				break;

			case Device_SATA:
				print("SATA\n");
				break;

			case Device_Unknowed:
				print("Unknow\n");
				break;
			}
		}
		if ((uint8_t)Detected_drives != Device_ATA)
		{
			cursor_color(0x0c);
			print("Wrong disk type for disk 0\n> Must be an ATA device\n");
			return;
		}
	}

	print("Getting partition info...\n");
	partition_t partitions[4];
	if (mbr_get_partition(partitions))
	{
		cursor_color(0x0c);
		print("Disk error\n");
		return;
	}

	for (uint8_t i = 0; i < 4; i++)
	{
		if (partitions[i].present)
		{
			print("partition ");
			echo(i + '0');
			print("\n  Start: ");
			hexprint32(partitions[i].LBA_start);
			print("\n  size: ");
			print_size_B(partitions[i].sector_count * 512);
			echo('\n');
		}
	}

	if (!partitions[0].present)
	{
		cursor_color(0x0c);
		print("Partition 1 of disk 0 is not present\n");
		return;
	}

	FAT_filesystem_t FAT_filesystem;
	FAT_init_partition(&partitions[0], &FAT_filesystem);

	ls("/", &FAT_filesystem);

	uint8_t local_keycode_register = *keycode_register;
	while (true)
	{
		while (*keycode_register != local_keycode_register)
		{
			echo(keycode_buffer[local_keycode_register]);
			local_keycode_register++;
		}
	}
	return;
}

void ls(char *path, FAT_filesystem_t *FAT_filesystem)
{
	uint32_t error = FAT_ls(path, FAT_filesystem);

	if (error)
	{
		cursor_color(0x0c);

		switch (error)
		{
		case 1:
			print("Folder not found\n");
			break;

		case 2:
			print("Not a directory\n");
			break;

		case 3:
			print("Invalid path\n");
			break;

		case 4:
			print("Disk error\n");
			break;

		default:
			print("Unknowed error");
			break;
		}

		cursor_color(0x0f);
	}
}