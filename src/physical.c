#include <mm/physical.h>
#include <common.h>

#define physical_pageAllocAddress( address ) ( physical_bitmap[BITMAP_BYTE_INDEX(address)] |= (1 << BITMAP_BIT_INDEX(address)) )

#define physical_pageBitIsSetForAddress( address ) ( physical_bitmap[BITMAP_BYTE_INDEX(address)] & (1 << BITMAP_BIT_INDEX(address)) )

// ядро начинается с 7c00
// Если положить в 0x7000 то
// на битмап будет 3072 байт, можно разметить 12 мб
static char * physical_bitmap = (char *)0x6000; 
static int physical_bitmapSize = SIZE_4KB;

// вообще physical_bitmapSize = SIZE_4KB позволяет разметить 128мб,
// но в bochs у меня 32мб, пусть будет 31 мб = 8191 страница
void * physical_maxAddress = (void *)(8191 * SIZE_4KB); //physical_bitmapSize * SIZE_4KB;

int physical_isPageFree(void * address){
	if (! physical_pageBitIsSetForAddress(address)){
		return SUCCESS;
	}
	return FAIL;
}

void * physical_pageAlloc() {
	void * physicalAddress = NULL;

	void * testAddr = 0;

	while (testAddr < physical_maxAddress){
		if (physical_isPageFree(testAddr) == SUCCESS){
			physical_pageAllocAddress(testAddr);
			return (void *)testAddr;
		}
		testAddr = (void *)((DWORD)testAddr + SIZE_4KB);
	}
	return NULL;
}

int physical_init(){
	// очищаем все страницы
	memset(physical_bitmap, 0x00, physical_bitmapSize);

	// помечаем место под битмап занятым
	physical_pageAllocAddress(physical_bitmap);

	physical_pageAllocAddress(0x0);

	// помечаем место под ядро занятым
	// см loader.asm KERNEL_SECTORS_NUM = 20
	// сектор 512 байт. 20 * 512 = 10240 = 0x2800
	// ядро занимает 2,5 страницы, конечный адрес ,
	//  округляем до 3
	physical_pageAllocAddress(0x7c00);
	physical_pageAllocAddress(0x8000);
	physical_pageAllocAddress(0x9000);

	return SUCCESS;
}

