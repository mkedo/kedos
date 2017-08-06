#ifndef _MM_PHYSICAL_H_
#define _MM_PHYSICAL_H_

#define SIZE_4KB 4096

#define BITMAP_BYTE_INDEX(address) ( ((DWORD)address/SIZE_4KB) / 8 )
#define BITMAP_BIT_INDEX(address) ( ((DWORD)address/SIZE_4KB) % 8 )

void * physical_pageAlloc();

#endif
