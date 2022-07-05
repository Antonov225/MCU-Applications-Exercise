/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include "ring_buffer.h"


bool RingBuffer_Init(RingBuffer *ringBuffer, char *dataBuffer, size_t dataBufferSize) 
{
	assert(ringBuffer);
	assert(dataBuffer);
	assert(dataBufferSize > 0);
	
	if ((ringBuffer) && (dataBuffer) && (dataBufferSize > 0)) {
	  ringBuffer->start=0;
	  ringBuffer->occupied=0;
	  ringBuffer->size=dataBufferSize;
	  ringBuffer->data=dataBuffer;
	  return true;
	}
	return false;
}

bool RingBuffer_Clear(RingBuffer *ringBuffer)
{
	assert(ringBuffer);
	
	if (ringBuffer) {
		for(uint16_t q=0;q<ringBuffer->size;q++){
			ringBuffer->data[q]=0;
		}
		ringBuffer->start=0;
		ringBuffer->occupied=0;
		return true;
	}
	return false;
}

bool RingBuffer_IsEmpty(const RingBuffer *ringBuffer)
{
    assert(ringBuffer);	
	
	if(ringBuffer->occupied!=0)return false;
	return true;
}

size_t RingBuffer_GetLen(const RingBuffer *ringBuffer)
{
	assert(ringBuffer);
	
	if (ringBuffer) {
		return ringBuffer->occupied;
	}
	return 0;
}

size_t RingBuffer_GetCapacity(const RingBuffer *ringBuffer)
{
	assert(ringBuffer);
	
	if (ringBuffer) {
		return ringBuffer->size;
	}
	return 0;	
}

bool RingBuffer_PutChar(RingBuffer *ringBuffer, char c)
{
	assert(ringBuffer);
	
	if (ringBuffer) {
        if(ringBuffer->occupied==ringBuffer->size)return false;
        ringBuffer->data[(ringBuffer->start+ringBuffer->occupied)%ringBuffer->size]=c;
        ringBuffer->occupied++;
		return true;
	}
	return false;
}

bool RingBuffer_GetChar(RingBuffer *ringBuffer, char *c)
{
	assert(ringBuffer);
	assert(c);
	
	if ((ringBuffer) && (c)) {
		if(ringBuffer->occupied==0)return false;
		*c = ringBuffer->data[ringBuffer->start];
		ringBuffer->start++;
		ringBuffer->occupied--;
		if(ringBuffer->start>=ringBuffer->size)ringBuffer->start=0;
		return true;
	}
	return false;
}
