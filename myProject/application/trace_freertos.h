
#ifndef _TRACE_H_
#define _TRACE_H_

#include <stdint.h>

/** 
 * Initializes pins that will be used as trace signals.
 * Initiallly, all signals are low.
 */
void TRACE_Init(void);

/**
 * Binds a trace, identified by a traceNumber with a given FreeRTOS task.
 * Once bound, the signal will be set high as long as the task is running.
 *
 * @param task FreeRTOS task handle (returned by xTaskCreate)
 * @param traceNumber trace number = 0 .. 7
 */
void TRACE_BindTaskWithTrace(void* task, int traceNumber);

/**
 * Sets all traces associated with FreeRTOS tasks to a low state.
 */
void TRACE_SetTaskTracesLow(void);

/**
 * Sets the given trace high.
 * 
 * @param traceNumber trace number = 0 .. 7
 */
void TRACE_SetHigh(int traceNumber);

/**
 * Sets the given trace low.
 * 
 * @param traceNumber trace number = 0 .. 7
 */
void TRACE_SetLow(int traceNumber);

/**
 * Toggles the given trace.
 * 
 * @param traceNumber trace number = 0 .. 7
 */
void TRACE_Toggle(int traceNumber);


/** 
 * The following macro is called by FreeRTOS whenever some task has been selected to run. 
 * At this point pxCurrentTCB contains the handle of the task about to enter the Running state.
 */
#define traceTASK_SWITCHED_IN()  													\
do {																											\
	TRACE_SetTaskTracesLow();																\
	if ((int)pxCurrentTCB->pxTaskTag >= 100)	{							\
		TRACE_SetHigh(((int)pxCurrentTCB->pxTaskTag)-100);		\
	}																												\
} while (0)

/**
 * The following macro is called inside the system tick ISR.
 * It will toggle trace signal 0 at every system tick.
 */
#define traceTASK_INCREMENT_TICK(xTickCount)	 TRACE_Toggle(0)


#endif // _TRACE_H_
