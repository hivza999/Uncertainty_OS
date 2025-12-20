#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

#define sector_size 512
#define sector_per_cluster 0x08

#define Cluster_size sector_per_cluster *sector_size
#define Cluster_shift 12
#define fat_nb_sector 0x0800

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_LABEL 0x08
#define ATTR_DIRECTORY 0x10

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

int add_entry(DirEntry_t *dir, char *name, uint8_t atttributes, uint32_t cluster_nb, uint32_t size);
void make_dir(DirEntry_t *dir, char *name, uint32_t *next_free_cluster);
uint32_t get_cluster_id(char *path);
int format_name(char *name, char formated_name[13]);

uint32_t FAT[fat_nb_sector * sector_size / sizeof(uint32_t)];
uint8_t Clusters[18][Cluster_size];

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
	for (uint8_t i; i < 8; i++)
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
	for (uint8_t i; i < 11; i++)
	{
		fat32_bios_parameter.volumme_name[i] = "UNCERTAINTY"[i];
	}
	for (uint8_t i; i < 8; i++)
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
	for (uint8_t i = 2; i < 6; i++)
	{
		fwrite(empty_sector, 512, 1, fptr); // 2-5
	}
	fwrite(&fat32_bios_parameter, 512, 1, fptr); // 6
	fwrite(&fat32_FSinfo, 512, 1, fptr);				 // 7
	for (uint8_t i = 8; i < fat32_bios_parameter.reserved_sector_count; i++)
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
		FILE *mkfs_ptr;
		char line[128];
		mkfs_ptr = fopen("tools_src/make_filesystem", "r");
		if (!mkfs_ptr)
		{
			printf("! Couldnt open file \"> make_filesystem\"\n");
			return (1);
		}

		uint32_t line_nb = 1;
		while (fgets(line, 100, mkfs_ptr))
		{
			uint32_t i = 2;
			switch (line[0])
			{
			case '\n':
				// empty line
				break;

			case 'd':
				while (line[i] != '\n')
				{
					i++;
				};
				while (i < 13)
				{
					line[i] = ' ';
					i++;
				}

				make_dir(root, &(line[2]), &next_free_cluster);
				break;

			case 'f':
				char path_source[64];
				uint16_t path_source_lentgh = 0;
				char path_destination[64];
				uint16_t path_destination_lentgh = 0;
				for (path_source_lentgh; path_source_lentgh < 7; path_source_lentgh++)
				{
					path_source[path_source_lentgh] = "bin/fs/"[path_source_lentgh];
				}

				i = 2;
				while (line[i] != ' ')
				{
					if (line[i] == '\n')
					{
						printf("! Expected at least 2 arguments on line %i\n", line_nb);
						return (1);
					}
					path_destination[path_destination_lentgh] = line[i];
					path_destination_lentgh++;
					i++;
				}
				path_destination[path_destination_lentgh] = 0;
				i++;

				while (line[i] != ' ' && line[i] != '\n')
				{
					path_source[path_source_lentgh] = line[i];
					path_source_lentgh++;
					i++;
				}

				// flags
				uint8_t flags = 0;
				if (line[i] == ' ')
				{
					i++;
					while (line[i] != '\n')
					{
						switch (line[i])
						{
						case ' ':
							break;

						case 's':
							flags |= ATTR_SYSTEM;
							break;

						default:
							printf("! Unknowed argument line %i: \"%c\"\n", line_nb, line[i]);
							return (1);
						}
						i++;
					}
				}

				path_source[path_source_lentgh] = 0;

				// get cluster id and file name
				char formated_name[13];

				int separator_index = path_destination_lentgh;
				while (path_destination[separator_index] != '/')
				{
					separator_index--;
					if (separator_index < 0)
					{
						printf("! Invalid location destination on line %i\n> %s\n", line_nb, path_destination);
					}
				}
				path_destination[separator_index] = 0;
				if (format_name(&(path_destination[separator_index + 1]), formated_name))
				{
					return (1);
				}
				FILE *file_source_ptr;
				file_source_ptr = fopen(path_source, "rb");
				if (!file_source_ptr)
				{
					printf("! Error, file not found: %s\n", path_source);
					return (1);
				}

				// get size
				struct stat st;
				stat(path_source, &st);
				uint16_t cluster_nb = ((((int)st.st_size) - 1) >> Cluster_shift) + 1;
				uint32_t cluster_id = next_free_cluster;

				// add entry to parent folder
				add_entry((DirEntry_t *)&(Clusters[get_cluster_id(path_destination)]), formated_name, flags, next_free_cluster, st.st_size);
				while (cluster_id + cluster_nb > next_free_cluster)
				{
					FAT[next_free_cluster] = next_free_cluster + 1;
					next_free_cluster++;
				}
				FAT[next_free_cluster - 1] = 0x0fffffff;

				fread(&(Clusters[cluster_id]), Cluster_size, cluster_nb, file_source_ptr);

				fclose(file_source_ptr);

				break;

			default:
				printf("! Error on line %i\n> Unknowed operation \"%c\"\n", line_nb, line[0]);
				return (1);
			}
			line_nb++;
		}

		fclose(mkfs_ptr);
	}

	// 2 copies are written
	fwrite(FAT, sizeof(FAT), 1, fptr);
	fwrite(FAT, sizeof(FAT), 1, fptr);

	fwrite(&(Clusters[2]), Cluster_size * 16, 1, fptr);

	printf("  Image builded successfully\n");
	return (0);
}

void make_dir(DirEntry_t *dir, char *name, uint32_t *next_free_cluster)
{
	uint32_t parent_cluster_nb;

	FAT[*next_free_cluster] = 0x0fffffff;
	add_entry(dir, name, ATTR_DIRECTORY, *next_free_cluster, 0);

	if (dir[0].Name == ".       ")
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
	add_entry(new_dir, ".          ", ATTR_DIRECTORY, *next_free_cluster, 0);
	add_entry(new_dir, "..         ", ATTR_DIRECTORY, parent_cluster_nb, 0);

	(*next_free_cluster)++;
}

int add_entry(DirEntry_t *dir, char *name, uint8_t atttributes, uint32_t cluster_nb, uint32_t size)
{
	int id = 0;
	while (dir[id].Name[0] != 0x00)
	{
		id++;
		if (id >= (Cluster_size / sizeof(DirEntry_t)))
		{
			printf("! Directory is full!\n");
		};
	};

	for (uint8_t i = 0; i < 11; i++)
	{
		dir[id].Name[i] = name[i];
	}
	dir[id].Attribute0 = atttributes;
	dir[id].cluster_nb_high = cluster_nb >> 16;
	dir[id].cluster_nb_low = cluster_nb;
	dir[id].size = size;
}

uint32_t get_cluster_id(char *path)
{
	printf("  Path: %s\n", path);
	return (3);
}

int format_name(char *name, char formated_name[13])
{
	printf("  name: %s\n", name);
	uint8_t i;
	for (i = 0; name[i] != '.' && name[i] != 0 && i < 9; i++)
	{
		if (i == 8)
		{
			printf("! Name too long\n> %s\n", name);
			return (1);
		}
		else if ((name[i] >= 0x21 && name[i] <= 0x29 && name[i] != 0x22) || (name[i] == 0x2d) || (name[i] >= 0x30 && name[i] <= 0x39) || (name[i] >= 0x40 && name[i] <= 0x5a) || (name[i] >= 0x5e && name[i] <= 0x60) || (name[i] >= 0x7b && name[i] <= 0x7e && name[i] != 0x7c))
		{
			formated_name[i] = name[i];
		}
		else
		{
			printf("! Invalid character in name\n> %s | char %i", name, i);
			formated_name[i] = '_';
		}
	}
	uint8_t j;
	for (j = i; j < 8; j++)
	{
		formated_name[j] = ' ';
	}

	if (name[i] == '.') // if extention is present
	{
		i++;
		for (i; name[i] != 0 && i < 12; i++)
		{
			if (i == 12)
			{
				printf("! Extention too long\n> %s\n", name);
				return (1);
			}
			else if ((name[i] >= 0x21 && name[i] <= 0x29 && name[i] != 0x22) || (name[i] == 0x2d) || (name[i] >= 0x30 && name[i] <= 0x39) || (name[i] >= 0x40 && name[i] <= 0x5a) || (name[i] >= 0x5e && name[i] <= 0x60) || (name[i] >= 0x7b && name[i] <= 0x7e && name[i] != 0x7c))
			{
				formated_name[j] = name[i];
			}
			else
			{
				printf("! Invalid character in name\n> %s | char %i", name, i);
				formated_name[j] = '_';
			}
			j++;
		}
	}

	for (j; j < 11; j++)
	{
		formated_name[j] = ' ';
	}
	return (0);
}
