#include "../display/text.h"
#include "../partition_table/partition.h"
#include "../storage/ata.h"
#include "fat.h"

int Read_Cluster(pio_read_packet_t *pio_read_packet, FAT_filesystem_t *filesystem, uint32_t Cluster_id);
int32_t get_cluster_id(char *path, FAT_filesystem_t *filesystem);
uint8_t lfn_checksum(char name[13]);

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_LABEL 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_LFN 0x0f

typedef struct LfnEntry
{
	uint8_t sequence_id;
	uint16_t name0[5];
	uint8_t attribute;
	uint8_t type;
	uint8_t checksum;
	uint16_t name1[6];
	uint16_t first_cluster;
	uint16_t name2[2];
} __attribute__((packed)) LfnEntry_t;

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
			print("Error reading disk while initialasing filesystem\n");
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
	/*
	1 > Folder not found
	2 > Not a directory
	3 > Invalid path
	4 > Disk error
	*/

	int32_t cluster_id = get_cluster_id(path, filesystem);

	if (cluster_id < 0)
	{
		return (-cluster_id);
	}

	DirEntry_t root_dir[filesystem->cluster_size * (512 / sizeof(DirEntry_t))];
	pio_read_packet_t pio_read_packet;
	pio_read_packet.sector_count = filesystem->cluster_size;
	pio_read_packet.buffer = root_dir;

	if (Read_Cluster(&pio_read_packet, filesystem, cluster_id))
	{
		while (1)
			;
	}

	char name[256];
	for (uint32_t i = 0; i < filesystem->cluster_size * (512 / sizeof(DirEntry_t)); i++)
	{
		if (root_dir[i].Attribute0 & ATTR_VOLUME_LABEL)
		{
			continue;
		}
		if (root_dir[i].Name[0] == 0x00)
		{
			break;
		}

		if (root_dir[i].Name[0] == 0xe5)
		{
			continue;
		}

		if (get_entry_name(root_dir, i, name))
		{
			continue;
		}

		print(name);
		if (root_dir[i].Attribute0 & ATTR_DIRECTORY)
		{
			echo('/');
		}

		echo('\n');
	}

	return (0);
}

int32_t get_cluster_id(char *path, FAT_filesystem_t *filesystem)
{
	/*
	return cluster id
	-1 > not found
	-2 > not a directory
	-3 > invalid path
	*/

	uint8_t attributes = ATTR_DIRECTORY; // to prevent interpreting files as directories

	DirEntry_t Dir_entries[filesystem->cluster_size * (512 / sizeof(DirEntry_t))];
	pio_read_packet_t pio_read_packet;
	pio_read_packet.buffer = &Dir_entries;
	pio_read_packet.sector_count = filesystem->cluster_size;

	char current_path[256];
	uint8_t curent_path_index = 0;
	uint32_t path_index = 1;
	int32_t cluster_id = filesystem->root_dir_cluster;

	if (path[0] != '/')
	{
		return (-3);
	}

	while (path[path_index] != 0)
	{
		if (!(attributes & ATTR_DIRECTORY))
		{
			return (-2);
		}

		while (path[path_index] != '/')
		{
			if (path[path_index] == 0)
			{
				path_index--;
				break;
			}

			current_path[curent_path_index] = path[path_index];
			path_index++;
			curent_path_index++;
		}
		current_path[curent_path_index] = 0;

		Read_Cluster(&pio_read_packet, filesystem, cluster_id);

		char name[256];
		for (uint32_t i = 0; i < filesystem->cluster_size * (512 / sizeof(DirEntry_t)); i++)
		{
			if (Dir_entries[i].Name[0] == 0)
			{
				return (-1);
			}
			if (Dir_entries[i].Name[0] == 0xe5)
			{
				continue;
			}
			if (Dir_entries[i].Attribute0 & ATTR_VOLUME_LABEL)
			{
				continue;
			}

			if (get_entry_name(Dir_entries, i, name))
			{
				continue;
			}

			uint32_t j = 0;
			bool found = false;
			while (current_path[j] == name[j])
			{
				if (current_path[j] == 0)
				{
					cluster_id = (Dir_entries[i].cluster_nb_high << 16) + Dir_entries[i].cluster_nb_low;
					found = true;
					break;
				}
				j++;
			}
			if (found)
			{
				break;
			}
		}

		path_index++;
		curent_path_index = 0;
	}
	return (cluster_id);
}

int get_entry_name(DirEntry_t *entries, int32_t id, char *name)
{
	if (id)
	{
		if (entries[id - 1].Attribute0 == ATTR_LFN && ((LfnEntry_t *)entries)[id - 1].checksum == lfn_checksum((char *)&entries[id]))
		{ // LFN detected

			int8_t offset = 1;
			uint8_t sequence_id = 0;
			uint16_t j = 0;

			while (((LfnEntry_t *)entries)[id - offset].sequence_id > sequence_id)
			{
				sequence_id = ((LfnEntry_t *)entries)[id - offset].sequence_id;

				for (uint32_t i = 0; i < 5; i++)
				{
					name[j] = (char)((LfnEntry_t *)entries)[id - offset].name0[i];
					if (!name[j])
					{
						return (0);
					}
					j++;
				}
				for (uint32_t i = 0; i < 6; i++)
				{
					name[j] = (char)((LfnEntry_t *)entries)[id - offset].name1[i];
					if (!name[j])
					{
						return (0);
					}
					j++;
				}
				for (uint32_t i = 0; i < 2; i++)
				{
					name[j] = (char)((LfnEntry_t *)entries)[id - offset].name2[i];
					if (!name[j])
					{
						return (0);
					}
					j++;
				}
				if (sequence_id & (1 << 6))
				{
					return (0);
				}
				offset++;
			}
			return (1);
		}
	}
	uint32_t i = 0;
	int32_t l = 8;
	uint8_t Case = (entries[id].Attribute1 & (1 << 3)) << 2; // 0x20 offset if the name is lowercase

	while (entries[id].Name[l - 1] == ' ')
	{
		l--;
	}

	uint8_t j = 0;
	for (j; j < l; j++)
	{
		if ('A' <= entries[id].Name[j] < 'Z')
		{
			name[i] = entries[id].Name[j] + Case;
		}
		else
		{
			name[i] = entries[id].Name[j];
		}
		i++;
	}

	// extention
	l = 3;
	while (entries[id].Extention[l - 1] == ' ' && l > 0)
	{
		l--;
	}

	if (l != 0)
	{
		Case = (entries[id].Attribute1 & (1 << 4)) << 1; // 0x20 offset if the extention is lowercase
		name[i] = '.';
		i++;
		j = 0;
		for (j; j < l; j++)
		{
			if ('A' <= entries[id].Extention[j] < 'Z')
			{
				name[i] = entries[id].Extention[j] + Case;
			}
			else
			{
				name[i] = entries[id].Extention[j];
			}
			i++;
		}
	}
	name[i] = 0;
	return (0);
}

int Read_Cluster(pio_read_packet_t *pio_read_packet, FAT_filesystem_t *filesystem, uint32_t Cluster_id)
{
	pio_read_packet->LBA = (filesystem->partition->LBA_start + filesystem->cluster_offset + filesystem->cluster_size * Cluster_id) | 0xe0000000;
	if (ATA_PIO_read(pio_read_packet))
	{
		print("Error reading cluster 0x");
		hexprint32(Cluster_id);
		echo('\n');
		return (1);
	}

	return (0);
}

uint8_t lfn_checksum(char name[13])
{
	uint8_t sum = 0;

	for (uint32_t i = 0; i < 11; i++)
	{
		sum = ((sum & 1) << 7) + (sum >> 1) + name[i];
	}

	return (sum);
}