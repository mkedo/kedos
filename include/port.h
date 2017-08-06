#ifndef _PORT_H_
#define _PORT_H_
#include <sys/types.h>

inline void port_outb( WORD port, BYTE data );

inline void port_outw( WORD port, WORD data );

inline void port_outd( WORD port, DWORD data );

inline BYTE port_inb( WORD port );

inline WORD port_inw( WORD port );

inline DWORD port_ind( WORD port );

#endif
