#include <stdint.h>

#define VIDEO_MEMORY 0xb8000
#define Scren_Width 80
#define Scren_Height 25

void print_d10(uint32_t value, uint8_t color, uint32_t *p_cursor);
void echo(char value, uint8_t color, uint32_t *p_cursor);
void print(char *string, uint8_t color, uint32_t *p_cursor);
void hexprint8(uint8_t value, uint8_t color, uint32_t *p_cursor);
void hexprint(uint8_t digit, uint8_t color, uint32_t *p_cursor);
void scroll(uint8_t lines);

void print_d10(uint32_t value, uint8_t color, uint32_t *p_cursor)
{
	char buffer[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	char i = 10;
	while (value > 0)
	{
		i--;
		buffer[i] = value % 10 + '0';
		value = value / 10;
	}

	print(buffer + i, color, p_cursor);
}

void hexprint8(uint8_t value, uint8_t color, uint32_t *p_cursor)
{
	hexprint(value >> 4, color, p_cursor);
	hexprint(value & 0x0f, color, p_cursor);
}

void hexprint(uint8_t digit, uint8_t color, uint32_t *p_cursor)
{
	if (digit < 10)
	{
		*(char *)(*p_cursor) = digit + '0';
		*(uint8_t *)(*p_cursor + 1) = color;
	}
	else
	{
		*(char *)(*p_cursor) = digit + ('a' - 10);
		*(uint8_t *)(*p_cursor + 1) = color;
	}
	*p_cursor += 2;
}

void print(char *string, uint8_t color, uint32_t *p_cursor)
{
	while (string[0])
	{
		echo(string[0], color, p_cursor);
		string++;
	}
}

void echo(char value, uint8_t color, uint32_t *p_cursor)
{
	*(char *)(*p_cursor + 1) = 0x0f;

	switch (value)
	{
	case (0x0a):
		*p_cursor += Scren_Width * 2 - ((*p_cursor - VIDEO_MEMORY) % (Scren_Width * 2));
		break;

	case (0x08):
		*p_cursor -= 2;
		*(char *)(*p_cursor) = 0x00;
		break;

	case (0x1b):
		*p_cursor = VIDEO_MEMORY;
		scroll(Scren_Height);
		break;

	default:
		*(char *)(*p_cursor) = value;
		*(uint8_t *)(*p_cursor + 1) = color;
		*p_cursor += 2;
	}

	if (*p_cursor >= VIDEO_MEMORY + Scren_Height * Scren_Width * 2)
	{
		*p_cursor -= Scren_Width * 2;
		scroll(1);
	}

	*(char *)(*p_cursor + 1) = 0xf0;
}

void scroll(uint8_t lines)
{
	uint32_t i;
	for (i = VIDEO_MEMORY; i < VIDEO_MEMORY + (Scren_Width * (Scren_Height - lines) * 2); i++)
	{
		*(uint8_t *)i = *(uint8_t *)(i + Scren_Width * lines * 2);
	}
	while (i < VIDEO_MEMORY + (Scren_Width * Scren_Height * 2))
	{
		*(uint8_t *)i = 0;
		i++;
	}
}