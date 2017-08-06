#include <terminal.h>
#include <sys/types.h>
#include <common.h>





static DWORD terminal_row;
static DWORD terminal_column;
static BYTE terminal_color;
static WORD* terminal_buffer;


static WORD inline make_vgaentry(char c, BYTE color)
{
	WORD c16 = c;
	WORD color16 = color;
	return c16 | color16 << 8;
}

void terminal_putentryat(char c, BYTE color, DWORD x, DWORD y)
{
	const DWORD index = y * VGA_WIDTH + x;
	terminal_buffer[index] = make_vgaentry(c, color);
}

void terminal_move_screen_up(){
	int y,x;
	for (y = 0; y < VGA_HEIGHT - 1; y++ ){
		for (x = 0; x < VGA_WIDTH; x++ )
		{
			const DWORD from = (y+1) * VGA_WIDTH + x;
			const DWORD to = y * VGA_WIDTH + x;
			terminal_buffer[to] = terminal_buffer[from];
		}
	}
	//creal last line
	y = VGA_HEIGHT - 1;
	for (x = 0; x < VGA_WIDTH; x++ ){
		const DWORD index = y * VGA_WIDTH + x;
		terminal_buffer[index] = make_vgaentry(' ', terminal_color);
	}
}
void terminal_new_line(){
	terminal_column = 0;
	++terminal_row;
	if (terminal_row >= VGA_HEIGHT){
		terminal_row = VGA_HEIGHT - 1;
		terminal_move_screen_up();
	}
}
void terminal_putchar(char c)
{
	if (c == 10){
		//new line
		terminal_new_line();
	} else {
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		++terminal_column;
		if (terminal_column >= VGA_WIDTH){
			terminal_new_line();
		}
	}
}

void terminal_writestring(const char * str)
{
	DWORD i;
	DWORD datalen = strlen(str);
	
	for (i = 0; i < datalen; ++i)
		terminal_putchar(str[i]);
}

void terminal_putint(int i, int base){
	static unsigned char buff[12];
	itoa(i, (unsigned char *)&buff, sizeof(buff), base);
	
	terminal_writestring((char *)&buff);
}

void terminal_init(){
	DWORD y,x;
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = 0x0C;
	terminal_buffer = (WORD*) 0xB8000;
	
	for (y = 0; y < VGA_HEIGHT; y++ ){
		for (x = 0; x < VGA_WIDTH; x++ )
		{
			const DWORD index = y * VGA_WIDTH + x;
			terminal_buffer[index] = make_vgaentry(' ', terminal_color);
			/*terminal_buffer[index+1] = (char)terminal_color;*/
			
		}
	}
}
