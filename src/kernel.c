#include <terminal.h>
#include <keyboard.h>
#include <common.h>
#include <kernel.h>

#include <mm/physical.h>

void kernel();

void _start() { kernel(); }


int global = 1;
void kernel() {	
	int i = 0;
	
	terminal_init();
	terminal_writestring("Terminal is inited\n");
	
	if (interrupt_init() == FAIL)
		kernel_panic("Can't initiate interrupts\n");


	terminal_writestring("global addr 0x");
	terminal_putint((int)&global, 16);
	terminal_putchar('\n');
	
	terminal_writestring("stack i 0x");
	terminal_putint((int)&i, 16);
	terminal_putchar('\n');
	
	terminal_writestring("func addr 0x");
	terminal_putint((int)&kernel, 16);
	terminal_putchar('\n');
	
	
	terminal_writestring("_start addr 0x");
	terminal_putint((int)&_start, 16);
	terminal_putchar('\n');
	
	__asm mov i, ebp;
	//ASM( "movl %%ebp, %0" : "=r" (i)  );
	
	terminal_writestring("stack base addr 0x");
	terminal_putint(i, 16);
	terminal_putchar('\n');
	__asm mov i, esp;
	//ASM( "movl %%esp, %0" : "=r" (i)  );
	
	terminal_writestring("stack pointer addr 0x");
	terminal_putint(i, 16);
	terminal_putchar('\n');
	
	keyboard_init();
	
	physical_init();
	void * pointer = physical_pageAlloc();
	
	terminal_writestring("allocated page at 0x");
	terminal_putint((DWORD)pointer, 16);
	terminal_putchar('\n');
	

	__asm sti;
	//ASM("STI");
	terminal_writestring("Ke:Do's Hello, World!\n");	
	terminal_writestring("Go to infinity loop\n");	

	for (;;);
}

void kernel_panic(const char * message) {
	__asm sti;
	terminal_writestring("Kernel Panic!");
	terminal_writestring(message);
	for (;;);
}
