#include <interrupt.h>
#include <sys/types.h>
#include <terminal.h>
#include <common.h>
#include <port.h>


char * interrupt_messages[] =
{
    "Divide By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
	
	"IRQ0",
	"IRQ1",
	"IRQ2",
	"IRQ3",
	"IRQ4",
	"IRQ5",
	"IRQ6",
	"IRQ7",
	
	"IRQ8",
	"IRQ9",
	"IRQ10",
	"IRQ11",
	"IRQ12",
	"IRQ13",
	"IRQ14",
	"IRQ15"
	
};

#define INTERRUPT_TABLE_ENTRIES 48
struct INTERRUPT_TABLE_POINTER interrupt_ptable;
struct INTERRUPT_TABLE_ENTRY interrupt_table[INTERRUPT_TABLE_ENTRIES];
INTERRUPT_HANDLER interrupt_handlers[INTERRUPT_TABLE_ENTRIES];

INTERRUPT_SERVICE_ROUTINE interrupt_stubs[] =
{
	isr00, isr01, isr02, isr03,	isr04, 
	isr05, isr06, isr07, isr08, isr09, 
	isr10, isr11, isr12, isr13, isr14,
	isr15, isr16, isr17, isr18, isr19, 
	isr20, isr21, isr22, isr23, isr24,
	isr25, isr26, isr27, isr28, isr29,
	isr30, isr31,
	
	isr32, isr33, isr34, isr35, isr36,
	isr37, isr38, isr39,
	
	isr40, isr41, isr42, isr43, isr44,
	isr45, isr46, isr47
	
};

void interrupt_dispatcher(struct interrupt_args * args){
	//static const char * numbers = "0123456789";
	int const interrupt_number = args->int_no;
	BOOL handled = FALSE;
	static char str_num[12];
	
	

	if (interrupt_number >=0 && interrupt_number < sizeof(interrupt_handlers)){
		if (interrupt_handlers[interrupt_number] != NULL){
			interrupt_handlers[interrupt_number](args);
			handled = TRUE;
		}
	}
	
	if (!handled){
		memset( str_num, 0, sizeof(str_num) );
		itoa(interrupt_number, (unsigned char *)str_num, sizeof(str_num), 10);
		
		if (interrupt_number != IRQ0){ // temporary don't show timer
			terminal_writestring("Unhandled interrupt  ");
			terminal_writestring(str_num);
		
		
			if (interrupt_number >= 0 && interrupt_number < sizeof(interrupt_messages)){
				terminal_putchar(' ');
				terminal_writestring(interrupt_messages[interrupt_number]);
			}
			
			terminal_writestring("\n");
		}
	}
	
	if (interrupt_number >= IRQ0 && interrupt_number <= IRQ7){
		irq_resume_master();
	} else if(interrupt_number >= IRQ8 && interrupt_number <= IRQ15){
		irq_resume_slave();
	}
	
}
int interrupt_set_handler(int index, INTERRUPT_HANDLER handler){
	if (index >= 0 && index < sizeof(interrupt_handlers)){
		interrupt_handlers[index] = handler;
		return SUCCESS;
	}
	return FAIL;
}

// http://wiki.osdev.org/8259_PIC
/*
In protected mode, the IRQs 0 to 7 conflict with the CPU exception which are reserved by Intel up until 0x1F. (It was an IBM design mistake.) Consequently it is difficult to tell the difference between an IRQ or an software error. It is thus recommended to change the PIC's offsets (also known as remapping the PIC) so that IRQs use non-reserved vectors. A common choice is to move them to the beginning of the available range (IRQs 0..0xF -> INT 0x20..0x2F). For that, we need to set the master PIC's offset to 0x20 and the slave's to 0x28. 
*/
void interrut_remap_pic(){
	unsigned char a1,a2;

	a1 = port_inb(MASTER_PIC_DATA);
	a2 = port_inb(SLAVE_PIC_DATA);
	
	port_outb( MASTER_PIC_CMD, 0x01 + 0x10 ); // starts the initialization sequence (in cascade mode)
    port_outb( SLAVE_PIC_CMD, 0x01 + 0x10 );
	
	port_outb( MASTER_PIC_DATA, 0x20 ); // Master PIC vector offset (IDT from ISR32 to IRS39)
    port_outb( SLAVE_PIC_DATA, 0x28 );  // Slave  PIC vector offset (IDT from ISR40 to IRS47)
	
	port_outb( MASTER_PIC_DATA, 0x04 ); // tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    port_outb( SLAVE_PIC_DATA, 0x02 );  // tell Slave PIC its cascade identity (0000 0010)
	
	port_outb( MASTER_PIC_DATA, 0x01 ); // 8086/88 (MCS-80/85) mode 
    port_outb( SLAVE_PIC_DATA, 0x01 );  // 8086/88 (MCS-80/85) mode 
	
	port_outb( MASTER_PIC_DATA, a1 ); 
    port_outb( SLAVE_PIC_DATA, a2 );  
	
}
int interrupt_init(){
	int index;
	
	interrupt_ptable.limit = ( sizeof(struct INTERRUPT_TABLE_ENTRY) * INTERRUPT_TABLE_ENTRIES) - 1;
	interrupt_ptable.base = (DWORD)&interrupt_table;
/*
	terminal_writestring("interrupt_ptable.base=0x");
	terminal_putint((int) & interrupt_table, 16);
	terminal_putchar('\n');
	*/
	for (index = 0; index < INTERRUPT_TABLE_ENTRIES; index++)
	{
		// init interrupt table enrty
		/*
		interrupt_table[index].base_high = ((DWORD)interrupt_stubs[index] & 0xFFFF0000) >> 16;
		interrupt_table[index].base_low  = ((DWORD)interrupt_stubs[index] & 0xFFFF);
		
		interrupt_table[index].present = 1;
		interrupt_table[index].DPL = RING0;
		interrupt_table[index].gate_type = 14;
		interrupt_table[index].storage_segment = 0;
		
		interrupt_table[index].reserved = 0;
		interrupt_table[index].unknown = 0;
		interrupt_table[index].selector = 0x08; //KERNEL_CODE_SEL
		*/
		
		
		interrupt_table[index].base_low = ((DWORD)interrupt_stubs[index] & 0xFFFF);
		interrupt_table[index].selector = 0x08;
		interrupt_table[index].always0 = 0;
		interrupt_table[index].flags = 0x8E;
		interrupt_table[index].base_high = ((DWORD)interrupt_stubs[index] & 0xFFFF0000) >> 16;
		
		/*
		terminal_writestring("idt=0x");
		terminal_putint(interrupt_table[index].flags, 16);
		terminal_writestring("\n");
		// init interrupt handler*/
		interrupt_handlers[index] = NULL;
	}
	
	
	
	interrut_remap_pic();

#ifdef _MSC_VER	
	__asm lidt interrupt_ptable;
#else
	ASM("lidt (%0)" : : "r" ( &interrupt_ptable));
#endif
	return SUCCESS;
}
