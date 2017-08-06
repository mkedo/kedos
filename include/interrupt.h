#ifndef _KERNEL_INTERRUPT_H_
#define _KERNEL_INTERRUPT_H_

#include "sys/types.h"

#define RING0		0
#define RING1		1
#define RING2		2
#define RING3		3

#define MASTER_PIC_CMD (0x20)
#define MASTER_PIC_DATA (0x21)
#define SLAVE_PIC_CMD (0xA0)
#define SLAVE_PIC_DATA (0xA1)

struct interrupt_args {
	unsigned int gs, fs, es, ds; 
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx,eax; /* pusha */
	unsigned int int_no, err_code;
	unsigned int eip, cs, eflags, useresp, ss; /* pushed by the processor automatically */
};

typedef void (* INTERRUPT_SERVICE_ROUTINE)();
typedef void (* INTERRUPT_HANDLER)( struct interrupt_args * args );


void interrupt_dispatcher(struct interrupt_args * args);
int interrupt_set_handler(int index, INTERRUPT_HANDLER handler);
int interrupt_init();

#ifdef _MSC_VER
#pragma pack(push,1)
#endif
struct INTERRUPT_TABLE_ENTRY
{
/*
    WORD base_low;
    WORD selector;
    BYTE reserved:5;
	BYTE unknown:3;
	BYTE gate_type:4; // 14 80386 32-bit interrupt gate
	BYTE storage_segment:1; // 0 for interrupt gates.
	BYTE DPL:2;
	BYTE present:1;
    WORD base_high;
*/
	WORD base_low; // The lower 16 bits of the address to jump to when this interrupt fires.
	WORD selector;  // Kernel segment selector.
	BYTE always0;  // This must always be zero.
	BYTE flags;
	WORD base_high; // The upper 16 bits of the address to jump to.
} PACKED;


struct INTERRUPT_TABLE_POINTER
{
    WORD limit;
    DWORD base;
} PACKED;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

enum { 
	INT0 = 0,
	INT1,
	INT2,
	INT3,
	INT4,
	INT5,
	INT6,
	INT7,
	INT8,
	INT9,
	INT10,
	INT11,
	INT12,
	INT13,
	INT14,
	INT15,
	INT16,
	INT17,
	INT18,
	INT19,
	INT20,
	INT21,
	INT22,
	INT23,
	INT24,
	INT25,
	INT26,
	INT27,
	INT28,
	INT29,
	INT30,
	INT31,
	
	IRQ0,
	IRQ1,
	IRQ2,
	IRQ3,
	IRQ4,
	IRQ5,
	IRQ6,
	IRQ7,
	
	IRQ8,
	IRQ9,
	IRQ10,
	IRQ11,
	IRQ12,
	IRQ13,
	IRQ14,
	IRQ15,
	SYSCALL_INTERRUPT
};

extern void irq_resume_master();
extern void irq_resume_slave();

extern void isr00();
extern void isr01();
extern void isr02();
extern void isr03();
extern void isr04();
extern void isr05();
extern void isr06();
extern void isr07();
extern void isr08();
extern void isr09();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();

extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();
#endif
