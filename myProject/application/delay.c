#include "stm32f4xx_hal.h"
#include "delay.h"

static volatile uint32_t SysTickCounter;
static uint32_t DelayCounter;
static uint32_t SysTickPrevious;

/**
 * System Tick Interrupt Service Routine
 */

void SysTick_Handler(void)
{
	SysTickCounter++;
}

void delay(unsigned int msDelay)
{
	SysTickPrevious = SysTickCounter;
	while (SysTickCounter < SysTickPrevious + msDelay){
		;
	}
} /* delay */