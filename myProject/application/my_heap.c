/* ================================================================================ *//**
 \license     %LICENSE%
 \file        heap.c
 \author      Lukasz Krzak
 \purpose     STM32 demo application
 \compiler    %COMPILER%
 \hardware    independent
 \doc         doxygen
 \brief       Simplest heap manager implementation for FreeRTOS
 \comments
*//* ================================================================================= */

#include <stdio.h>
#include "FreeRTOS.h"


// -----------------------------------------------------------------------------
//  pvPortMalloc
// -----------------------------------------------------------------------------
void *pvPortMalloc( size_t xWantedSize )
{
	static uint8_t heap[configTOTAL_HEAP_SIZE];
	static size_t index;
	void* result = NULL;

	if (index+xWantedSize < sizeof(heap)) {
		result = (void*)(heap+index);
		index += xWantedSize;
		// make sure that index is aligned to 32bits
		index += 4-(index % 4);
		printf("allocated %d bytes\n", xWantedSize);
	} else {
		printf("failed to allocate %d bytes\n", xWantedSize);
	}

	return result;

} /* pvPortMalloc */


// -----------------------------------------------------------------------------
//  vPortFree
// -----------------------------------------------------------------------------
void vPortFree( void *pv )
{

} /* vPortFree */