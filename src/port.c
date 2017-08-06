#include <port.h>
#include <sys/types.h>

extern
inline void port_outb( WORD port, BYTE data )
{
#ifdef _MSC_VER
// on x86 the "out" instruction requires the port and the outputting value 
// to be placed into (e)dx and {eax/ax/al} registers respectively.
	__asm
	{
		mov al, data;
		mov dx, port;
		out dx, al;
	}
#else
	ASM( "outb %%al, %%dx" :: "d" (port), "a" (data) );
#endif
}

extern
inline void port_outw( WORD port, WORD data )
{
#ifdef _MSC_VER
	__asm
	{
		mov ax, data;
		mov dx, port;
		out dx, ax;
	}
#else
	ASM( "outw %%ax, %%dx" :: "d" (port), "a" (data) );
#endif
}
extern
inline void port_outd( WORD port, DWORD data )
{
#ifdef _MSC_VER
	__asm
	{
		mov eax, data;
		mov dx, port;
		out dx, eax;
	}
#else
	ASM( "outl %%eax, %%dx" :: "d" (port), "a" (data) );
#endif
}
extern
inline BYTE port_inb( WORD port )
{
	BYTE data;
#ifdef _MSC_VER
	__asm {
		mov dx, port;
		in al, dx;
		mov data, al;
	}
#else
	ASM( "inb %%dx, %%al" : "=a" (data) : "d" (port) );
#endif
	return data;
}
extern
inline WORD port_inw( WORD port )
{
	WORD data;
#ifdef _MSC_VER
	__asm {
		mov dx, port;
		in ax, dx;
		mov data, ax;
	}
#else
	ASM( "inw %%dx, %%ax" : "=a" (data) : "d" (port) );
#endif
	return data;
}
extern
inline DWORD port_ind( WORD port )
{
	DWORD data;
#ifdef _MSC_VER
	__asm {
		mov dx, port;
		in eax, dx;
		mov data, eax;
	}
#else
	ASM( "inl %%dx, %%eax" : "=a" (data) : "d" (port) );
#endif
	return data;
}
