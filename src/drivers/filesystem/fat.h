#pragma once
#include "../partition_table/partition.h"
#include "filesystem.h"

typedef struct
{
	uint32_t fat_offset;
	uint32_t cluster_offset;
	uint32_t cluster_size;
	uint32_t root_dir_cluster;
	partition_t partition;
} FAT_filesystem_t;

typedef struct
{
	uint32_t Cluster_id;
	uint32_t index;
} FAT_Directory_t;

int FAT_opendir(filesystem_t *filesystem, Directory_t *Directory, char *path);
int FAT_readdir(filesystem_t *filesystem, Directory_t *Directory, dir_entry_t *dir_entry);
