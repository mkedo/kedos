format MS COFF
use32
align 4

KERNEL_DATA_SEL = 0x10

extrn '_interrupt_dispatcher' as interrupt_dispatcher
public irq_resume_master as '_irq_resume_master'
public irq_resume_slave as '_irq_resume_slave'

MACRO ISR_A num {
public _isr#num
_isr#num:
	push dword num
	jmp _isr_common
}

MACRO ISR_NO_ERRCODE num {
public _isr#num
_isr#num:
	push dword 0 ; err code
	push dword num
	jmp _isr_common
}


_isr_common:	
	pusha
	push ds
	push es
	push fs
	push gs

	
	mov ax, KERNEL_DATA_SEL
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	
	mov eax, esp
	push eax
	call interrupt_dispatcher
	
	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	;pushad
	;mov eax, [esp + 32]
	;push eax
	;call interrupt_dispatcher
	;add esp, 4
	;popad
	;add esp, 4
	iret
	
irq_resume_master:
	push eax
	mov al, 0x20
	out 0x20, al
	pop eax
	ret
	
irq_resume_slave:
	push eax
	mov al, 0x20
	out 0xA0, al
	out 0x20, al
	pop eax
	ret

ISR_NO_ERRCODE 00
ISR_NO_ERRCODE 01
ISR_NO_ERRCODE 02
ISR_NO_ERRCODE 03
ISR_NO_ERRCODE 04
ISR_NO_ERRCODE 05
ISR_NO_ERRCODE 06
ISR_NO_ERRCODE 07
ISR_A 08
ISR_NO_ERRCODE 09
ISR_A 10
ISR_A 11
ISR_A 12
ISR_A 13
ISR_A 14
ISR_NO_ERRCODE 15
ISR_NO_ERRCODE 16
ISR_NO_ERRCODE 17
ISR_NO_ERRCODE 18
ISR_NO_ERRCODE 19
ISR_NO_ERRCODE 20
ISR_NO_ERRCODE 21
ISR_NO_ERRCODE 22
ISR_NO_ERRCODE 23
ISR_NO_ERRCODE 24
ISR_NO_ERRCODE 25
ISR_NO_ERRCODE 26
ISR_NO_ERRCODE 27
ISR_NO_ERRCODE 28
ISR_NO_ERRCODE 29
ISR_NO_ERRCODE 30
ISR_NO_ERRCODE 31

; irq master
ISR_NO_ERRCODE 32
ISR_NO_ERRCODE 33
ISR_NO_ERRCODE 34
ISR_NO_ERRCODE 35
ISR_NO_ERRCODE 36
ISR_NO_ERRCODE 37
ISR_NO_ERRCODE 38
ISR_NO_ERRCODE 39
; irq slave
ISR_NO_ERRCODE 40
ISR_NO_ERRCODE 41
ISR_NO_ERRCODE 42
ISR_NO_ERRCODE 43
ISR_NO_ERRCODE 44
ISR_NO_ERRCODE 45
ISR_NO_ERRCODE 46
ISR_NO_ERRCODE 47
