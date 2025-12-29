#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#define sector_size 512
#define sector_per_cluster 0x08

#define Cluster_size sector_per_cluster *sector_size
#define Cluster_shift 12
#define fat_nb_sector 0x0800
#define Cluster_count 4096

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_LABEL 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_LFN 0x0f

typedef struct fat32_bios_parameter
{
	// boot record
	uint8_t bootjmp[3];
	char identifier[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sector_count;
	uint8_t nb_fat;
	uint16_t root_entry_count;
	uint16_t unused0;
	uint8_t media_type;
	uint16_t unused1;
	uint16_t sectors_per_track;
	uint16_t head;
	uint32_t hidden_sector_count;
	uint32_t sector_count;

	// Extended boot record
	uint32_t fat_sector_count;
	uint16_t flags;
	uint16_t fat_version;
	uint32_t nb_root_cluster;
	uint16_t fsinfo_sector;
	uint16_t backup_boot_record_sector;
	uint8_t reserved0[12];
	uint8_t nb_drive;
	uint8_t reserved1;
	uint8_t signature0;
	uint32_t volume_ID;
	char volumme_name[11];
	char system_iditenfier[8];

	uint8_t unused2[0x1a4];
	uint16_t signature1;

} __attribute__((packed)) fat32_bios_parameter_t;

typedef struct fat32_FSinfo
{
	uint32_t signature0;
	uint8_t reserved0[480];
	uint32_t signature1;
	uint32_t last_free_cluster;
	uint32_t first_free_cluster;
	uint8_t reserved1[12];
	uint32_t signature3;

} __attribute__((packed)) fat32_FSinfo_t;

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

void make_dir(DirEntry_t *dir, char *name, uint32_t *next_free_cluster);
void add_entry(DirEntry_t *dir, char *name, uint8_t atttributes0, uint32_t cluster_nb, uint32_t size);

void add_lfn_entry(LfnEntry_t *dir, char name[13], uint8_t checksum, uint8_t sequence_id);
uint8_t lfn_checksum(char name[11]);

int format_name(char *name, char formated_name[11]);
int copy_dir(char *path, uint32_t *cluster, uint32_t *next_free_cluster);

uint32_t FAT[fat_nb_sector * sector_size / sizeof(uint32_t)];
uint8_t Clusters[Cluster_count + 2][Cluster_size];

int main()
{
	FILE *fptr;
	fptr = fopen("bin/part/partition", "wb");
	if (fptr == NULL)
	{
		printf("! Couldn't open destination file\n");
		return (1);
	}
	printf("  Building FAT32 image...\n");

	// Reserved sectors
	fat32_bios_parameter_t fat32_bios_parameter;
	fat32_FSinfo_t fat32_FSinfo;
	uint8_t empty_sector[512];

	for (uint16_t i = 0; i < 512; i++)
	{
		((uint8_t *)&fat32_bios_parameter)[i] = 0x00;
		((uint8_t *)&fat32_FSinfo)[i] = 0x00;
		empty_sector[i] = 0;
	}

	// BPB
	fat32_bios_parameter.bootjmp[0] = 0xeb;
	fat32_bios_parameter.bootjmp[1] = 0xfe;
	fat32_bios_parameter.bootjmp[2] = 0x90;
	for (uint32_t i; i < 8; i++)
	{
		fat32_bios_parameter.identifier[i] = "UncertFS"[i];
	}
	fat32_bios_parameter.bytes_per_sector = sector_size;
	fat32_bios_parameter.sectors_per_cluster = sector_per_cluster;
	fat32_bios_parameter.reserved_sector_count = 0x0020;
	fat32_bios_parameter.nb_fat = 2;
	fat32_bios_parameter.root_entry_count = 0;
	fat32_bios_parameter.media_type = 0xf8;
	fat32_bios_parameter.sectors_per_track = 0x003e;
	fat32_bios_parameter.head = 0x00f7;
	fat32_bios_parameter.hidden_sector_count = 0x00000800;
	fat32_bios_parameter.sector_count = 0x001ffffe;

	fat32_bios_parameter.fat_sector_count = fat_nb_sector;
	fat32_bios_parameter.flags = 0x0000;
	fat32_bios_parameter.fat_version = 0x0000;
	fat32_bios_parameter.nb_root_cluster = 2;
	fat32_bios_parameter.fsinfo_sector = 0x0001;
	fat32_bios_parameter.backup_boot_record_sector = 0x0006;
	fat32_bios_parameter.nb_drive = 0x80;
	fat32_bios_parameter.signature0 = 0x29;
	fat32_bios_parameter.volume_ID = 0x0000abcd;
	for (uint32_t i = 0; i < 11; i++)
	{
		fat32_bios_parameter.volumme_name[i] = "UNCERTAINTY"[i];
	}
	for (uint32_t i = 0; i < 8; i++)
	{
		fat32_bios_parameter.system_iditenfier[i] = "FAT32   "[i];
	}

	// FSinfo
	fat32_FSinfo.signature0 = 0x41615252;
	fat32_FSinfo.signature1 = 0x61417272;
	fat32_FSinfo.last_free_cluster = -1;
	fat32_FSinfo.first_free_cluster = -1;
	fat32_FSinfo.signature3 = 0xaa550000;

	// write it
	fwrite(&fat32_bios_parameter, 512, 1, fptr); // 0
	fwrite(&fat32_FSinfo, 512, 1, fptr);				 // 1
	for (uint32_t i = 2; i < 6; i++)
	{
		fwrite(empty_sector, 512, 1, fptr); // 2-5
	}
	fwrite(&fat32_bios_parameter, 512, 1, fptr); // 6
	fwrite(&fat32_FSinfo, 512, 1, fptr);				 // 7
	for (uint32_t i = 8; i < fat32_bios_parameter.reserved_sector_count; i++)
	{
		fwrite(empty_sector, 512, 1, fptr);
	}

	// fat
	for (uint32_t i = 0; i < fat_nb_sector * sector_size / sizeof(uint32_t); i++)
	{
		FAT[i] = 0;
	}

	FAT[0] = 0x0ffffff8;
	FAT[1] = 0x0fffffff;

	// clear cluster
	for (uint32_t i = 0; i < sizeof(Clusters); i++)
	{
		((uint8_t *)Clusters)[i] = 0;
	}

	DirEntry_t *root;
	root = (DirEntry_t *)&Clusters[2];
	FAT[2] = 0x0fffffff;

	add_entry(root, "UNCERTAINTY", ATTR_VOLUME_LABEL, 0, 0);
	uint32_t next_free_cluster = 3;

	{ // make filesystem

		make_dir(root, "boot", &next_free_cluster); // add BOOT directory

		// Add the kernel to it
		FILE *file_source_ptr;
		file_source_ptr = fopen("bin/part/kernel", "rb");
		if (!file_source_ptr)
		{
			printf("! Error, file not found: %s\n", "bin/part/kernel");
			return (1);
		}

		// get size
		struct stat st;
		stat("bin/part/kernel", &st);
		uint16_t cluster_nb = ((((uint32_t)st.st_size) - 1) >> Cluster_shift) + 1;
		uint32_t cluster_id = next_free_cluster;

		// add entry to parent folder
		add_entry((DirEntry_t *)&(Clusters[3]), "kernel", ATTR_SYSTEM, next_free_cluster, st.st_size);
		while (cluster_id + cluster_nb > next_free_cluster)
		{
			FAT[next_free_cluster] = next_free_cluster + 1;
			next_free_cluster++;
		}
		FAT[next_free_cluster - 1] = 0x0fffffff;

		fread(&(Clusters[cluster_id]), Cluster_size, cluster_nb, file_source_ptr);
		fclose(file_source_ptr);

		// copy other files
		char path[256];
		for (uint32_t i = 0; i < 8; i++)
		{
			path[i] = "bin/fs/"[i];
		}
		uint32_t clusters[64];
		clusters[0] = 2;

		if (copy_dir(path, &(clusters[0]), &next_free_cluster))
		{
			return (1);
		}
	}

	// 2 copies are written
	fwrite(FAT, sizeof(FAT), 1, fptr);
	fwrite(FAT, sizeof(FAT), 1, fptr);

	fwrite(&(Clusters[2]), Cluster_size, Cluster_count, fptr);

	printf("  Image builded successfully\n");
	return (0);
}

int copy_dir(char *path, uint32_t *cluster, uint32_t *next_free_cluster)
{
	DIR *d;
	struct dirent *dir;
	d = opendir(path);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_name[0] != '.')
			{
				if (dir->d_type == DT_DIR) // directory
				{
					*(cluster + sizeof(uint32_t)) = *next_free_cluster;

					make_dir((DirEntry_t *)(&Clusters[*cluster]), dir->d_name, next_free_cluster);

					uint32_t i = 0;
					uint32_t len = strlen(path);
					while (dir->d_name[i] != 0)
					{
						path[i + len] = dir->d_name[i];
						i++;
					}
					path[i + len] = '/';
					path[i + len + 1] = 0;

					if (copy_dir(path, cluster + sizeof(uint32_t), next_free_cluster))
					{
						return (1);
					}

					path[len] = 0;
				}

				else if (dir->d_type == DT_REG) // file
				{
					uint32_t i = 0;
					uint32_t len = strlen(path);
					while (dir->d_name[i] != 0)
					{
						path[i + len] = dir->d_name[i];
						i++;
					}
					path[i + len] = 0;

					// get cluster id and file name
					char formated_name[13];

					FILE *file_source_ptr;
					file_source_ptr = fopen(path, "rb");
					if (!file_source_ptr)
					{
						printf("! Error, file not found: %s\n", path);
						return (1);
					}

					// get size
					struct stat st;
					stat(path, &st);
					uint32_t cluster_nb;
					if (st.st_size)
					{
						cluster_nb = ((((uint32_t)st.st_size) - 1) >> Cluster_shift) + 1;
					}
					else
					{
						cluster_nb = 1;
					}
					uint32_t cluster_id = *next_free_cluster;

					// add entry to parent folder
					add_entry((DirEntry_t *)&(Clusters[*cluster]), dir->d_name, 0, *next_free_cluster, st.st_size);
					while (cluster_id + cluster_nb > *next_free_cluster)
					{
						FAT[*next_free_cluster] = (*next_free_cluster) + 1;
						(*next_free_cluster)++;
					}
					FAT[*next_free_cluster - 1] = 0x0fffffff;

					fread(&(Clusters[cluster_id]), Cluster_size, cluster_nb, file_source_ptr);

					fclose(file_source_ptr);

					path[len] = 0;
				}
			}
		}
		closedir(d);
	}
	else
	{
		printf("! Couldnt open %s\n", path);
		return (1);
	}
	return (0);
}

void make_dir(DirEntry_t *dir, char *name, uint32_t *next_free_cluster)
{
	uint32_t parent_cluster_nb;

	FAT[*next_free_cluster] = 0x0fffffff;
	add_entry(dir, name, ATTR_DIRECTORY, *next_free_cluster, 0);

	if (*((uint64_t *)&(dir[0].Name)) == 0x202020202020202e) // ASCII of ".       " in a uint64 form
	{
		parent_cluster_nb = dir[0].cluster_nb_high << 16 + dir[0].cluster_nb_low;
	}
	else
	{
		if (dir[0].Attribute0 == ATTR_VOLUME_LABEL)
		{
			parent_cluster_nb = 0;
		}
		else
		{
			printf("! Error,\n> Invalid parent directory\n");
		}
	}

	DirEntry_t *new_dir;
	new_dir = (DirEntry_t *)&Clusters[*next_free_cluster];
	add_entry(new_dir, ".", ATTR_DIRECTORY, *next_free_cluster, 0);
	add_entry(new_dir, "..", ATTR_DIRECTORY, parent_cluster_nb, 0);

	(*next_free_cluster)++;
}

void add_entry(DirEntry_t *dir, char *name, uint8_t atttributes, uint32_t cluster_nb, uint32_t size)
{
	uint32_t id = 0;
	while (dir[id].Name[0] != 0x00)
	{
		id++;
		if (id >= (Cluster_size / sizeof(DirEntry_t)))
		{
			printf("! Directory is full!\n");
		};
	};

	char formated_name[13];
	uint8_t flags;

	if (atttributes == ATTR_VOLUME_LABEL)
	{
		flags = 0;

		for (uint32_t i = 0; i < 11; i++)
		{
			dir[id].Name[i] = name[i];
		}
	}
	else
	{
		flags = format_name(name, formated_name);
		if (flags == 1)
		{
			uint8_t checksum = lfn_checksum(formated_name);
			uint32_t len = strlen(name) / 13 + 1;

			add_lfn_entry((LfnEntry_t *)dir, &(name[len * 13 - 13]), checksum, 0x40 + len);
			for (int32_t i = len - 2; i >= 0; i--)
			{
				add_lfn_entry((LfnEntry_t *)dir, &(name[i * 13]), checksum, i + 1);
			}
			id += len;

			flags = 0;
		}

		for (uint32_t i = 0; i < 11; i++)
		{
			dir[id].Name[i] = formated_name[i];
		}
	}

	dir[id].Attribute0 = atttributes;
	dir[id].Attribute1 = flags;
	dir[id].cluster_nb_high = cluster_nb >> 16;
	dir[id].cluster_nb_low = cluster_nb;
	dir[id].size = size;
}

void add_lfn_entry(LfnEntry_t *dir, char name[13], uint8_t checksum, uint8_t sequence_id)
{
	uint32_t id = 0;
	while (dir[id].sequence_id != 0x00)
	{
		id++;
		if (id >= (Cluster_size / sizeof(LfnEntry_t)))
		{
			printf("! Directory is full!\n");
		};
	};

	dir[id].sequence_id = sequence_id;
	dir[id].attribute = 0x0f;
	dir[id].checksum = checksum;

	for (uint32_t i = 0; i < 5; i++)
	{
		dir[id].name0[i] = name[i];
	}
	for (uint32_t i = 0; i < 6; i++)
	{
		dir[id].name1[i] = name[i + 5];
	}
	for (uint32_t i = 0; i < 2; i++)
	{
		dir[id].name2[i] = name[i + 11];
	}
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

int format_name(char *name, char formated_name[11])
{
	uint8_t flags = 0x06;
	// case flags
	// bit 0 > lfn
	// bit 1 & 2 > undefined
	// bit 3 & 4 > the one that will be used

	// please keep only 1 . in file name

	if (!strcmp(name, "."))
	{
		for (uint32_t i = 0; i < 11; i++)
		{
			formated_name[i] = ".          "[i];
		}
		return (0);
	}

	if (!strcmp(name, ".."))
	{
		for (uint32_t i = 0; i < 11; i++)
		{
			formated_name[i] = "..         "[i];
		}
		return (0);
	}

	uint32_t i;

	for (i = 0; name[i] != '.' && name[i] != 0 && i < 9; i++)
	{
		if (i == 8)
		{
			// Name too long
			flags = 1;
		}
		else if ((name[i] >= 0x21 && name[i] <= 0x29 && name[i] != 0x22) || (name[i] == 0x2d) || (name[i] >= 0x30 && name[i] <= 0x39) || (name[i] == 0x40) || (name[i] >= 0x5e && name[i] <= 0x60) || (name[i] >= 0x7b && name[i] <= 0x7e && name[i] != 0x7c))
		{
			formated_name[i] = name[i];
		}
		else if (name[i] >= 0x41 && name[i] <= 0x5a)
		{
			if (flags & 0x02)
			{
				flags = 0x04; // upper case name, undefined extention
			}
			else if (flags & 0x08)
			{
				// Combination of lower and upper case
				flags = 1;
			}

			formated_name[i] = name[i];
		}
		else if (name[i] >= 0x61 && name[i] <= 0x7a)
		{
			if (flags & 0x02)
			{
				flags = 0x0c; // lower case name, undefined extention
			}
			else if (!(flags & 0x08))
			{
				// Combination of lower and upper case
				flags = 1;
			}

			formated_name[i] = name[i] - 0x20;
		}
		else
		{
			// Invalid character in name
			formated_name[i] = '_';
			flags = 1;
		}
	}
	if (flags & 0x02)
	{
		flags = 0x04; // if no letter > uppercase ame + undefined extention
	}
	if (i == 9)
	{
		formated_name[6] = '~';
		formated_name[7] = '1';
		while (name[i] != '.' && name[i] != 0)
		{
			i++;
		}
	}

	uint32_t j;
	for (j = i; j < 8; j++)
	{
		formated_name[j] = ' ';
	}
	j = 8;

	if (name[i] == '.') // if extention is present
	{
		i++;
		for (i; name[i] != 0 && j < 13; i++)
		{
			if (j == 12)
			{
				// Extention too long
				flags = 1;
			}
			else if ((name[i] >= 0x21 && name[i] <= 0x29 && name[i] != 0x22) || (name[i] == 0x2d) || (name[i] >= 0x30 && name[i] <= 0x39) || (name[i] == 0x40) || (name[i] >= 0x5e && name[i] <= 0x60) || (name[i] >= 0x7b && name[i] <= 0x7e && name[i] != 0x7c))
			{
				formated_name[j] = name[i];
			}
			else if (name[i] >= 0x41 && name[i] <= 0x5a)
			{
				if (flags & 0x04)
				{
					flags = (flags & 0x08); // upper case extention
				}
				else if (flags & 0x10)
				{
					// Combination of lower and upper case
					flags = 1;
				}

				formated_name[j] = name[i];
			}
			else if (name[i] >= 0x61 && name[i] <= 0x7a)
			{
				if (flags & 0x04)
				{
					flags = (flags & 0x08) | 0x10; // lower case extention
				}
				else if (!(flags & 0x10))
				{
					// Combination of lower and upper case
					flags = 1;
				}

				formated_name[j] = name[i] - 0x20;
			}
			else
			{
				// Invalid character in name
				formated_name[j] = '_';
				flags = 1;
			}
			j++;
		}
	}
	if (flags & 0x04)
	{
		flags &= 0x08;
	}

	for (j; j < 11; j++)
	{
		formated_name[j] = ' ';
	}

	return (flags);
}
