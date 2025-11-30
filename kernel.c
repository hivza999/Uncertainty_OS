#define VIDEO_MEMORY 0xb8000

void print(char *string, char color);

int Cursor;

extern void main()
{
	Cursor = VIDEO_MEMORY;

	char *message = "Hello World!";
	print(message, 0x0f);
	return;
}

void print(char *string, char color)
{
	while (string[0])
	{
		*(char *)(Cursor) = string[0];
		*(char *)(Cursor + 1) = color;
		Cursor += 2;
		*(char *)(Cursor + 1) = 0x10; // cursor color
		string++;
	}
}
