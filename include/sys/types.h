#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#define TRUE				0x01
#define FALSE				0x00

#define SUCCESS				0
#define FAIL				-1

#ifdef _MSC_VER
	#define PACKED 
#else
	#define ASM					__asm__ __volatile__
	#define PACKED				__attribute__( (packed) )
#endif

#ifdef _MSC_VER
	#define inline __inline
#endif

#ifndef NULL
#define NULL				((void *)0x00000000)
#endif

#ifdef _MSC_VER
typedef __int8				BOOL;
typedef unsigned __int8		BYTE;
typedef unsigned __int16	WORD;
typedef unsigned __int32	DWORD;
#else
typedef int					BOOL;
typedef unsigned char		BYTE;
typedef unsigned short		WORD;
typedef unsigned long		DWORD;
#endif
#define SIZE_BYTE			1
#define SIZE_WORD			2
#define SIZE_DWORD			4

#endif
