#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "interrupt.h"

#define KEYBORAD_PORT_CMD (0x64)
#define KEYBORAD_PORT_DATA (0x60)

void keyboard_handler(struct interrupt_args *args);
void keyboard_init();

#endif
