#include <keyboard.h>
#include <sys/types.h>
#include <terminal.h>
#include <port.h>
#include <common.h>


// http://www.acm.uiuc.edu/sigops/roll_your_own/hardware/kb.html
// http://www.osdever.net/bkerndev/Docs/keyboard.htm

/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */
/*
ALT 0x38
R ALT 0xE0 0x38
CAPS LOCK 0x3A
F1-F10 0x3B - 0x44
NUM LOCK 0x45
SCROLL LOCK 0x46
HOME 0x47
UP 0x48
PG UP 0x49
LEFT 0x4B
NUM 5 0x4C
RIGHT 0x4D
END 0x4F
DOWN 0x50
PG DOWN 0x51
INSERT 0x52
DEL 0x53
F11 0x57
F12 0x58
LEFT WIN 0xE0 0x5B
RIGHT WIN 0xE0 0x5C
CTX MENU 0xE0 0x5D
*/
static unsigned char keyboard_layout[128] =
{
 /*         0   1    2    3    4    5    6    7    8    9    A    B    C    D   E    F  */
 /*0x0 */   0,  0, '1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', 0, '\t',
 /*0x1 */ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	0, 'a', 's',
 /*0x2 */ 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`',  0,   0,  'z', 'x','c', 'v',
 /*0x3 */ 'b', 'n', 'm', ',', '.', '/',  0,	'*',   0,  ' ',	 0,	  0,   0,   0,   0,   0,
 /*0x4 */  0,   0,   0,   0,   0,	0,	 0,	  0,   0,	0,	'-',  0,   0,   0,  '+',  0,
 /*0x5 */  0,   0,	 0,	  0,   0,   0,   0,   0,   0,	0,	
};	
static BOOL ext_scancode = FALSE;
void keyboard_handler(struct interrupt_args *args){
	unsigned char scancode_txt[12];
	BYTE scancode;
	BYTE keycode;
	BOOL released;
	
	scancode = port_inb(KEYBORAD_PORT_DATA);
	keycode = scancode & ~0x80;
	
	terminal_writestring("scan code 0x");
	itoa((int)scancode, (unsigned char *)&scancode_txt, sizeof(scancode_txt), 16);
	terminal_writestring((char *)&scancode_txt);
	terminal_putchar(' ');
	
	if (scancode == 0xE0 || scancode == 0x60){
		// two bites code
		ext_scancode = TRUE;
		terminal_putchar('\n');
		return;
	}
	if (scancode == 0 || scancode > 0x79 || scancode == 0x61){
		
	}
	
	//memset(scancode_hex, 0, 12);
	itoa((int)scancode & 0x7F, (unsigned char *)&scancode_txt, sizeof(scancode_txt), 16);
	terminal_writestring("key code 0x");
	terminal_writestring((char *)&scancode_txt);
	terminal_putchar(' ');
	
	//itoa((int)scancode & 0x7F, (unsigned char *)&scancode_txt, sizeof(scancode_txt), 10);
	//terminal_writestring((char *)&scancode_txt);
	
	
	
	
	released = ((scancode & 0x80) == 0x80);
	if (released)
		terminal_writestring("RELEASED ");
	else 
		terminal_writestring("PRESSED ");
	
	
	// http://lxr.free-electrons.com/source/include/dt-bindings/input/input.h#L54
	
	if (keycode < sizeof(keyboard_layout) && keyboard_layout[keycode] != 0){
		terminal_writestring("char ");
		terminal_putchar('\'');
		terminal_putchar(keyboard_layout[keycode]);
		terminal_putchar('\'');
	} else {
	
		if (keycode == 0x2A){
			terminal_writestring("<LEFT SHIFT> ");
		} else if (keycode == 0x36){
			terminal_writestring("<RIGHT SHIFT> ");
		} else if (keycode == 0x1D){
			if (ext_scancode){
				terminal_writestring("<RIGHT CONTROL> ");
			} else {
				terminal_writestring("<LEFT CONTROL> ");
			}
		} else if (keycode == 0x38){
			if (ext_scancode){
				terminal_writestring("<RIGHT ALT> ");
			} else {
				terminal_writestring("<LEFT ALT> ");
			}
		} else if (keycode == 0x01){
			terminal_writestring("<ESC> ");
		} else if (keycode == 0x5B && ext_scancode){
			terminal_writestring("<LEFT WIN> ");
		} else if (keycode == 0x5C && ext_scancode){
			terminal_writestring("<RIGHT WIN> ");
		} else {
			terminal_writestring("UNKNOWN CONTROL KEY ");
		}
	}
	
	terminal_putchar('\n');
	ext_scancode = FALSE;
}
void keyboard_init(){
	interrupt_set_handler(IRQ1, keyboard_handler);
}
