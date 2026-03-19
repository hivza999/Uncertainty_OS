#pragma once
#include <stdint.h>

typedef struct
{
	uint32_t type;
	void *filesystem_data;
} filesystem_t;

typedef struct
{
	void *buffer;
	uint32_t buffer_size;
} Directory_t;

typedef struct
{
	char *name;
	uint8_t type; // bit 0 -> directory
} dir_entry_t;

#define dir_entry_ATTR_DIR 1

int filesystem_init(partition_t *partition);

int opendir(Directory_t *Directory, char *path);
int readdir(Directory_t *Directory, dir_entry_t *dir_entry);
int fread(void *buffer, uint32_t size, char *path);
