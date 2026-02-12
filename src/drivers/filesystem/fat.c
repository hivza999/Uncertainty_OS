#include "../partition_table/partition.h"
#include "../storage/ata.h"
#include "fat.h"
#include "filesystem.h"

#define FAT_ATTR_READ_ONLY 0x01
#define FAT_ATTR_HIDDEN 0x02
#define FAT_ATTR_SYSTEM 0x04
#define FAT_ATTR_VOLUME_LABEL 0x08
#define FAT_ATTR_DIRECTORY 0x10
#define FAT_ATTR_LFN 0x0f

typedef struct
{
	char Name[8];
	char Extention[3];
	uint8_t Attribute0;
	uint8_t Attribute1;
	uint8_t create_time_ms;
	uint16_t create_time;
	uint16_t create_date;
	uint16_t access_date;
	uint16_t cluster_nb_high;
	uint16_t modified_time;
	uint16_t modified_date;
	uint16_t cluster_nb_low;
	uint32_t size;

} __attribute__((packed)) FAT_Dir_Entry_t;

typedef struct
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

int Read_Cluster(void *buffer, FAT_filesystem_t *filesystem, uint32_t Cluster_id);
int get_entry_name(FAT_Dir_Entry_t *entries, int32_t id, char *name);
int32_t get_cluster_id(char *path, FAT_filesystem_t *filesystem);
uint8_t lfn_checksum(char name[13]);

uint32_t FAT_get_DIR_buffer_size(filesystem_t *filesystem)
{
	return (((FAT_filesystem_t *)filesystem->filesystem_data)->cluster_size * 512 + sizeof(FAT_Directory_t));
}

int FAT_opendir(filesystem_t *filesystem, Directory_t *Directory, char *path)
{
	/*
	3 > buffer size too small
	4 > No such file or directory
	5 > Not a directory
	*/

	if (Directory->buffer_size < FAT_get_DIR_buffer_size(filesystem))
	{
		return (3);
	}

	((FAT_Directory_t *)Directory->buffer)->index = 0;
	((FAT_Directory_t *)Directory->buffer)->Cluster_id = get_cluster_id(path, (FAT_filesystem_t *)filesystem->filesystem_data);

	if ((int32_t)((FAT_Directory_t *)Directory->buffer)->Cluster_id < 0)
	{
		switch ((int32_t)((FAT_Directory_t *)Directory->buffer)->Cluster_id)
		{
		case -1:
			return (4);
		case -2:
			return (5);
		case -3:
			return (4);
		default:
			return (1);
		}
	}
	return (0);
}

int FAT_readdir(filesystem_t *filesystem, Directory_t *Directory, dir_entry_t *dir_entry)
{
	/*
	3 > Disk error
	*/

	FAT_Directory_t *FAT_directory = (FAT_Directory_t *)(Directory->buffer);
	if (FAT_directory->index == 0)
	{
		if (Read_Cluster((Directory->buffer) + sizeof(FAT_Directory_t), (FAT_filesystem_t *)filesystem->filesystem_data, (((FAT_Directory_t *)Directory->buffer)->Cluster_id)))
		{
			return (3);
		}
	}

	FAT_Dir_Entry_t *Entries = Directory->buffer + sizeof(FAT_Directory_t);

	if (Entries[FAT_directory->index].Name[0] == 0)
	{
		dir_entry->name[0] = 0;
		return (0);
	}

	while (Entries[FAT_directory->index].Attribute0 & FAT_ATTR_VOLUME_LABEL)
	{
		if (Entries[FAT_directory->index].Name[0] == 0)
		{
			return (0);
		}
		FAT_directory->index++;
	}

	if (get_entry_name(Entries, FAT_directory->index, dir_entry->name))
	{
		return (1);
	}

	dir_entry->type = 0;
	if (Entries[FAT_directory->index].Attribute0 & FAT_ATTR_DIRECTORY)
	{
		dir_entry->type |= dir_entry_ATTR_DIR;
	}

	FAT_directory->index++;
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

	uint8_t attributes = FAT_ATTR_DIRECTORY; // to prevent interpreting files as directories

	FAT_Dir_Entry_t Dir_entries[filesystem->cluster_size * (512 / sizeof(FAT_Dir_Entry_t))];
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
		if (!(attributes & FAT_ATTR_DIRECTORY))
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

		if (Read_Cluster(&Dir_entries, filesystem, cluster_id))
		{
			return (-3);
		}

		char name[256];
		for (uint32_t i = 0; i < filesystem->cluster_size * (512 / sizeof(FAT_Dir_Entry_t)); i++)
		{
			if (Dir_entries[i].Name[0] == 0)
			{
				return (-1);
			}
			if (Dir_entries[i].Name[0] == 0xe5)
			{
				continue;
			}
			if (Dir_entries[i].Attribute0 & FAT_ATTR_VOLUME_LABEL)
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

int get_entry_name(FAT_Dir_Entry_t *entries, int32_t id, char *name)
{
	if (id)
	{
		if (entries[id - 1].Attribute0 == FAT_ATTR_LFN && ((LfnEntry_t *)entries)[id - 1].checksum == lfn_checksum((char *)&entries[id]))
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

int Read_Cluster(void *buffer, FAT_filesystem_t *filesystem, uint32_t Cluster_id)
{
	/*
	1 > Error reading
	*/

	pio_read_packet_t pio_read_packet;
	pio_read_packet.buffer = buffer;
	pio_read_packet.LBA = (filesystem->partition.LBA_start + filesystem->cluster_offset + filesystem->cluster_size * Cluster_id) | 0xe0000000;
	pio_read_packet.sector_count = filesystem->cluster_size;
	pio_read_packet.sector_count = 1;

	if (ATA_PIO_read(&pio_read_packet))
	{
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
