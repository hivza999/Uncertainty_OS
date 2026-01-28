#include "../display/text.h"
#include "../partition_table/partition.h"
#include "../storage/ata.h"
#include "fat.h"

int Read_Cluster(pio_read_packet_t *pio_read_packet, partition_t *partition, FAT_filesystem_t *filesystem, uint32_t Cluster_id);
void print_name_fat_entry(DirEntry_t entry, uint8_t color);

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_LABEL 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_LFN 0x0f

typedef struct
{
	uint8_t Data[4096];
} Cluster_t;

int FAT_init_partition(partition_t *partition, FAT_filesystem_t *filesystem)
{
	{ // get the filesystem info
		uint8_t buffer[512];

		pio_read_packet_t pio_read_packet;
		pio_read_packet.LBA = partition->LBA_start | 0xe0000000;
		pio_read_packet.sector_count = 1;
		pio_read_packet.buffer = buffer;

		if (ATA_PIO_read(&pio_read_packet))
		{
			print("Error reading disk while initialasing filesystem\n", 0x0f);
			while (1)
				;
		}

		// in sector
		filesystem->cluster_size = buffer[0x0d];
		filesystem->fat_offset = *((uint16_t *)&buffer[0x0e]);
		filesystem->cluster_offset = buffer[0x10] * *((uint32_t *)&buffer[0x24]) + filesystem->fat_offset - filesystem->cluster_size * 2;
		filesystem->root_dir_cluster = *((uint32_t *)&buffer[0x2c]);
		filesystem->partition = partition;
	}

	return (0);
}

int FAT_ls(char *path, FAT_filesystem_t *filesystem)
{
	DirEntry_t root_dir[filesystem->cluster_size * (512 / sizeof(DirEntry_t))];
	pio_read_packet_t pio_read_packet;
	pio_read_packet.LBA = filesystem->partition->LBA_start | 0xe0000000;
	pio_read_packet.sector_count = filesystem->cluster_size;
	pio_read_packet.buffer = root_dir;

	if (Read_Cluster(&pio_read_packet, filesystem->partition, filesystem, filesystem->root_dir_cluster))
	{
		while (1)
			;
	}

	print("\n> ", 0x0f);
	print(path, 0x0f);
	echo('\n', 0x0f);

	for (uint32_t i = 0; i < filesystem->cluster_size * (512 / sizeof(DirEntry_t)); i++)
	{
		if (root_dir[i].Attribute0 & ATTR_VOLUME_LABEL)
		{
			continue;
		}
		if (root_dir[i].Name[0] == 0x00)
		{
			i = (1 << 24);
			continue;
		}

		if (root_dir[i].Name[0] == 0xe5)
			continue;

		print_name_fat_entry(root_dir[i], 0x0f);
	}
}

void print_name_fat_entry(DirEntry_t entry, uint8_t color)
{
	int32_t l = 8;
	while (entry.Name[l - 1] == ' ')
	{
		l--;
	}

	uint8_t i = 0;
	for (i; i < l; i++)
	{
		echo(entry.Name[i], color);
	}

	// extention
	l = 3;
	while (entry.Extention[l - 1] == ' ' && l > 0)
	{
		l--;
	}

	if (l != 0)
	{
		echo('.', color);
		i = 0;
		for (i; i < l; i++)
		{
			echo(entry.Extention[i], color);
		}
	}
	if (entry.Attribute0 & ATTR_DIRECTORY)
	{
		echo('/', color);
	}

	echo('\n', color);
	return;
}

int Read_Cluster(pio_read_packet_t *pio_read_packet, partition_t *partition, FAT_filesystem_t *filesystem, uint32_t Cluster_id)
{
	pio_read_packet->LBA = (partition->LBA_start + filesystem->cluster_offset + filesystem->cluster_size * Cluster_id) | 0xe0000000;
	if (ATA_PIO_read(pio_read_packet))
	{
		print("Error reading cluster 0x", 0x0f);
		hexprint32(Cluster_id, 0x0f);
		echo('\n', 0x0f);
		return (1);
	}

	return (0);
}
