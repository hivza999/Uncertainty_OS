#include <stdint.h>
#include "fat.h"
#include "../storage/ata.h"
#include "../partition_table/partition.h"
#include "filesystem.h"

#define FS_none 0
#define FS_FAT 1

static filesystem_t filesystem;
static uint8_t filesystem_data[64];

int filesystem_init(partition_t *partition)
{
	/*
	2 > no filesystem found
	3 > Error reading disk while initialasing filesystem
	*/

	filesystem.filesystem_data = &filesystem_data;

	// get the filesystem info
	uint8_t buffer[512];

	pio_read_packet_t pio_read_packet;
	pio_read_packet.LBA = partition->LBA_start | 0xe0000000;
	pio_read_packet.sector_count = 1;
	pio_read_packet.buffer = buffer;

	if (ATA_PIO_read(&pio_read_packet))
	{
		return (3);
	}

	if (*(uint64_t *)&buffer[0x52] == 0x2020203233544146) // "FAT32   " ascii, fat32 signature (not always here but here will always be here)
	{
		FAT_filesystem_t *FAT_filesystem_ptr = (FAT_filesystem_t *)&filesystem_data;

		filesystem.type = FS_FAT;

		// in sector
		FAT_filesystem_ptr->cluster_size = buffer[0x0d];
		FAT_filesystem_ptr->fat_offset = *((uint16_t *)&buffer[0x0e]);
		FAT_filesystem_ptr->cluster_offset = buffer[0x10] * *((uint32_t *)&buffer[0x24]) + FAT_filesystem_ptr->fat_offset - FAT_filesystem_ptr->cluster_size * 2;
		FAT_filesystem_ptr->root_dir_cluster = *((uint32_t *)&buffer[0x2c]);
		FAT_filesystem_ptr->partition.LBA_start = partition->LBA_start;
		FAT_filesystem_ptr->partition.partition_id = partition->partition_id;
		FAT_filesystem_ptr->partition.present = partition->present;
		FAT_filesystem_ptr->partition.sector_count = partition->sector_count;

		return (0);
	}

	filesystem.filesystem_data = FS_none;
	return (2);
}

int opendir(Directory_t *Directory, char *path)
{
	/*
	2 > No filesystem
	3 > buffer size too small
	4 > No such file or directory
	5 > Not a directory
	*/
	if (filesystem.type == FS_FAT)
	{
		return (FAT_opendir(&filesystem, Directory, path));
	}
	return (2);
}

int readdir(Directory_t *Directory, dir_entry_t *dir_entry)
{
	/*
	2 > No filesystem
	3 > Disk error

	dir_entry->name will be empty when reached the end of the direcotry
	*/

	if (filesystem.type == FS_FAT)
	{
		return (FAT_readdir(&filesystem, Directory, dir_entry));
	}
	return (2);
}

int fread(void *buffer, uint32_t size, char *path)
{
	/*
	No filesystem
	*/

	if (filesystem.type == FS_FAT)
	{
		return (FAT_fread(&filesystem, buffer, path, size));
	}
	return (2);
}