#pragma once
#include "../partition_table/partition.h"

typedef struct
{
	uint32_t fat_offset;
	uint32_t cluster_offset;
	uint32_t cluster_size;
	uint32_t root_dir_cluster;
	partition_t *partition;
} FAT_filesystem_t;

typedef struct DirEntry
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

} __attribute__((packed)) DirEntry_t;

int FAT_init_partition(partition_t *partition, FAT_filesystem_t *filesystem);
int FAT_ls(char *path, FAT_filesystem_t *filesystem);
