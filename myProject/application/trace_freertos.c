#include "trace_freertos.h"              // module's header file
#include "stm32f4xx_hal.h"			// STM32Cube HAL include file
#include "FreeRTOS.h"           // FreeRTOS main include file
#include "task.h"               // FreeRTOS include file


typedef struct 
{
	GPIO_TypeDef *GPIO;
	uint16_t      pin;
} TraceSignal;


static TraceSignal traceSignal[8] = { {GPIOC, GPIO_PIN_14},    // trace signal 0
                                      {GPIOC, GPIO_PIN_15},    // trace signal 1
                                      {GPIOF, GPIO_PIN_0},     // trace signal 2
                                      {GPIOF, GPIO_PIN_1},     // trace signal 3
                                      {GPIOF, GPIO_PIN_2},     // trace signal 4
                                      {GPIOF, GPIO_PIN_3},     // trace signal 5
                                      {GPIOF, GPIO_PIN_4},     // trace signal 6
                                      {GPIOF, GPIO_PIN_5}      // trace signal 7
                                    }; 

static uint8_t TRACE_TaskMask = 0x00;

void TRACE_Init(void)
{
	int i;
  GPIO_InitTypeDef GPIO_InitStruct;

  // GPIOC Ports Clock Enable
  __GPIOC_CLK_ENABLE();
	// GPIOF Ports Clock Enable
  __GPIOF_CLK_ENABLE();
	
	for (i = 0; i < 8; i++) {
		// Configure GPIO pin
		GPIO_InitStruct.Pin   = traceSignal[i].pin;
		GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;				// push-pull output
		GPIO_InitStruct.Pull  = GPIO_NOPULL;                // pull-down disabled
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;             // analog pin bandwidth limited
		HAL_GPIO_Init(traceSignal[i].GPIO, &GPIO_InitStruct);		
		// Set pin to a low state
		HAL_GPIO_WritePin(traceSignal[i].GPIO, traceSignal[i].pin, GPIO_PIN_RESET);
	}	
} /* TRACE_Init */


void TRACE_BindTaskWithTrace(void * task, int traceNumber)
{
	/* Set the application tag for a task. This tag identifies the trace number
	   associated with the task. Since the trace number starts from 0 we want to
	   distinguish empty tag from zero trace - thus the tags start from 100 */
	vTaskSetApplicationTaskTag(task, (TaskHookFunction_t)(traceNumber+100));
	
	// include this trace in the task trace mask
	TRACE_TaskMask |= (1<<traceNumber);
	
} /* TRACE_BindTaskWithTrace */


void TRACE_SetTaskTracesLow(void)
{
	int i;
	
	for (i = 0; i < 8; i++) {
		if (TRACE_TaskMask & (1<<i)) {
			// Set pin to a low state
		  HAL_GPIO_WritePin(traceSignal[i].GPIO, traceSignal[i].pin, GPIO_PIN_RESET);
		}
	}
} /* TRACE_SetTaskTracesLow */


void TRACE_SetHigh(int traceNumber)
{	
	// set signal to a high state
	HAL_GPIO_WritePin(traceSignal[traceNumber].GPIO, traceSignal[traceNumber].pin, GPIO_PIN_SET);
	
} /* TRACE_SetHigh */


void TRACE_SetLow(int traceNumber)
{
	// set signal to a low state
	HAL_GPIO_WritePin(traceSignal[traceNumber].GPIO, traceSignal[traceNumber].pin, GPIO_PIN_RESET);
	
} /* TRACE_SetLow */


void TRACE_Toggle(int traceNumber)
{
	// toggle signal
	HAL_GPIO_TogglePin(traceSignal[traceNumber].GPIO, traceSignal[traceNumber].pin);
	
} /* TRACE_Toggle */
