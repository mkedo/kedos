;
; Kedos 2013
;
format binary

use16
org 7C00h
KERNEL_SECTORS_NUM = 20
SECTORS_TO_LOAD = 2 + KERNEL_SECTORS_NUM

; --------
; Точка входа
; --------
Start:
	xor ax,ax
	mov ss, ax
	mov ds, ax
	mov sp, 0x7C00
	
	mov [drive_num], dl; save drive num
	
	jmp loader

welcome_msg	db	"Ke:Do's Loader", 13, 10, 0
loaderror_msg db "Load sector error :(",  13, 10, 0
reseterror_msg db "Can't reset drive", 13, 10, 0
drive_num db 0x0

; --------
; Печатает сообщение
; DS:SI адрес строки
; --------
Print: 
	lodsb ;1 байт в регистр AL по адресу DS:SI 
	or	al, al
	jz	PrintDone
	
	mov	ah, 0eh ; Teletype output. 
	            ; AL = Character, BH = Page Number, 
				; BL = Color (only in graphic mode)
	int	10h     ; Video display functions (including VESA/VBE)
	jmp	Print
PrintDone:
	ret

; --------
; --------
loader:

	
	mov ah, 0  ; Set video mode
	mov al, 3h ; Video mode: 80x25 16/8
	int 10h    ; Video display functions (including VESA/VBE)
	
	mov ah, 02h ; Set cursor position
	mov bh, 0   ; BH = Page Number
	mov dx, 0   ; DH = Row, DL = Column
	int 10h     ; Video display functions (including VESA/VBE)

	mov	si, welcome_msg
	call	Print
	

	
	mov ecx, 3 ; 3 попытки
try_reset:
	sub ecx, 1
	test ecx, ecx
	je reseterror
	;----------------------
	; Reset Disk Drive
	mov		ah, 0   		; AH=00h: Reset Disk Drive
	mov		dl, [drive_num] ; DL Drive. drive 0 is floppy drive
	int		0x13			; Reset Disk Drive
	                		; CF Set on error
	jc try_reset
	
	
	;----------------------
	; Читаем 2 сектор с дискеты
	mov		ax, 7E0h
	mov		es, ax	; ES:BX	Buffer Address Pointer
	xor		bx, bx 
	
	mov		ah, 0x02			; AH=02h: Read Sectors From Drive
	mov		al, SECTORS_TO_LOAD	; AL=Sectors To Read Count
	mov		ch, 0				; CH=Track
	mov		cl, 2				; CL=Sector
	mov		dh, 0				; DH=Head
	mov		dl, [drive_num] 	; DL=Drive. Remember Drive 0 is floppy drive.
readsector:
	int		0x13	; call BIOS - Read the sector
					; CF	Set On Error, Clear If No Error
					; AL	Actual Sectors Read Count
	jc readsector
	cmp al, SECTORS_TO_LOAD
	jne loaderror
	

	
	
	; Прыгаем во второй сектор
	jmp 7E0h:00h
	
reseterror:
	mov si, reseterror_msg
	call	Print
loaderror:
	mov si, loaderror_msg
	call	Print
shutdown:	
	xor	ax, ax; ah - Read Character
	int	0x16 ; Read Character
	int 0x19 ; reboot
	cli
	hlt

times 510 - ($-$$) db 0
dw 0xAA55








; --------
; Второй сектор
; --------
use16
org 7E00h ; 7C00h + 512
	mov si, jump_to_protect_msg
	call Print2
	
	
	; Скрываем курсор. Он нам больше не понадобиться
	
	mov ah, 01h ; Set text-mode cursor shape
	            ; CH = Scan Row Start, CL = Scan Row End
	xor cx,cx
	or ch, 00010000b ; if bit 5 of CH is set, that often means "Hide cursor"
	int 10h

	cli
	; Загружаем GDT
	lgdt [gdt_desc]
	
	; Открываем шлюз А20
	in al, 0x92 
	or al, 2
	out 0x92, al
	
	; Включаем Защищенный режим
	
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	
	
	; Прыжок в 32-битный код
	jmp 0x08:protected_start

	
; ****************************
; GDT
; ****************************
gdt_data:
	dd 0 ; null descriptor
	dd 0
; gdt code 0x08
	dw 0FFFFh    ; limit low
	dw 0         ; base low
	db 0         ; base middle
	db 10011010b ; access
	db 11001111b ; granularity
	db 0         ; base high
	
; gdt data 0x10
	dw 0FFFFh    ; limit low
	dw 0         ; base low
	db 0         ; base middle
	db 10010010b ; access
	db 11001111b ; granularity
	db 0         ; base high
end_of_gdt:
gdt_desc:
	dw end_of_gdt - gdt_data - 1 ; limit (Size of GDT)
	dd gdt_data                  ; base of GDT
	
	
; ****************************

; ****************************
Print2: 
	lodsb ;1 байт в регистр AL по адресу DS:SI 
	or	al, al
	jz	PrintDone2

	mov	ah, 0eh ; Teletype output. 
	            ; AL = Character, BH = Page Number, 
				; BL = Color (only in graphic mode)
	int	10h     ; Video display functions (including VESA/VBE)
	jmp	Print2
	
PrintDone2:
	ret
	
	
	
jump_to_protect_msg db "Try to jump to protect mode", 13,10,0
times 512 - ($-$$) db 0

use32
; ****************************
; Третий сектор
; ****************************
protected_start:
	
	mov ax, 0x10
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov esp, 90000h
	mov ebp, 90000h
	
	mov edi, 0x0B8000
	mov byte[edi+0], 'W'
	mov byte[edi+1], 06h
	
	mov byte[edi+2], 'o'
	mov byte[edi+3], 06h
	mov byte[edi+4], 'r'
	mov byte[edi+5], 06h
	mov byte[edi+6], 'k'
	mov byte[edi+7], 06h
	mov byte[edi+8], 's'
	mov byte[edi+9], 06h
	mov byte[edi+10], '!'
	mov byte[edi+11], 06h
	
	jmp 0x08:kernel_main 

stop:
	cli
	hlt
	
times 1024 - ($-$$) db 0
; ****************************
; Четвертый сектор. Ядро 0x8200
; ****************************
kernel_main:
	
;times 2048 - ($-$$) db 0
