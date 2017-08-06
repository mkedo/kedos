#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <sys/types.h>


#define VGA_WIDTH (80)
#define VGA_HEIGHT (24)

void terminal_putentryat(char c, BYTE color, DWORD x, DWORD y);
void terminal_putchar(char c);
void terminal_writestring(const char * str);
void terminal_init();
void terminal_putint(int i, int base);
#endif
