#include "stm32f4xx_hal.h"
#include "delay.h"
#include <stdio.h>
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace_freertos.h"
#include "semphr.h"

extern volatile uint32_t SysTickCounter;
static uint32_t DelayCounter;
static uint32_t SysTickPrevious;

typedef struct {
        GPIO_TypeDef* gpio;    // GPIO port
        uint16_t      pin;     // GPIO pin
        TickType_t    ticks;   // delay expressed in system ticks
} BlinkParams;

//SemaphoreHandle_t myMutex;
SemaphoreHandle_t mySemaphore;

//LEDs Init

void LED_Init(void)
{
	  // GPIO Ports Clock Enable
    __GPIOG_CLK_ENABLE();
	
    GPIO_InitTypeDef GPIO_InitStruct;

    // Configure GPIO pin PG.13 and PG.14
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // push-pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL; // no pull
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW; // analog pin bandwidth limited
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
	
} //LED_Init

//Task USART

void taskUSART(void* params)
{
    while (1) {
				char buf;
				if(USART_GetChar(&buf)){
					USART_PutChar(buf);
				}
        // introduce some delay
        vTaskDelay(10);
    } // while (1)
}

//TaskLED

void taskLED(void* params)
{
  // Blink the LED based on the provided arguments
  while (params) {
    // Toggle LED
    HAL_GPIO_TogglePin(((BlinkParams*)params)->gpio, ((BlinkParams*)params)->pin);
    // Introduce some delay
    vTaskDelay(((BlinkParams*)params)->ticks);
  } // while (params)
 
  // Delete the task in case the provided argument is NULL
  vTaskDelete(NULL);
 
} //taskLED

void taskLED_semaphore(void* params){
	while(1){
		if(mySemaphore != NULL){
			if(xSemaphoreTake(mySemaphore, ( TickType_t ) 10 ) == pdTRUE){
			// Toggle LED
			HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
			}
		}
	}
} //taskLEDsemaphore

//-----------------------------------------------------------------------


void activity(void)
{
		static volatile uint32_t period = 400000;
    volatile uint32_t i;
        
    // this loop executes 400000 or 80000 times,
    // which makes this activity once longer once shorter in time
    for (i = 0; i < period; i++) {
        __asm__ volatile ("NOP");
    }
        
    if (period == 400000) {
        period = 80000;
    } else {
        period = 400000;
    }
}

void taskDELAY(void *params)
{
		TickType_t start, stop;
		
		// two trace signals
		TRACE_SetLow(5);
		TRACE_SetLow(6);
		
		while(1){
			// trace signal 5 toggles on every loop
			TRACE_Toggle(5);
			// save activity start time
			start = xTaskGetTickCount();
			// trace 6 goes hugh at the start of activity
			TRACE_SetHigh(6);
			activity();
			// trace 6 goes low at the end of activity
			TRACE_SetLow(6);
			// save activity end time
			stop = xTaskGetTickCount();
			// delay task execution until 500 ms - duration of activity
			// vTaskDelay(500 - (stop - start));
			vTaskDelayUntil(&start, 500);
	}
}


/**
 * Configures EXTI Line2 (connected to PG2 pin) in interrupt mode
 */
static void EXTI2_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
        
  // Enable GPIOG clock
  __GPIOG_CLK_ENABLE();
  
  // Configure PG2 pin as input with EXTI interrupt on the falling edge and pull-up
  GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
 
  // Enable and set EXTI Line2 Interrupt to the lowest priority
  HAL_NVIC_SetPriority(EXTI2_IRQn, 15, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}
 
/**
 * This function handles External line 2 interrupt request.
 */
void EXTI2_IRQHandler(void)
{	
  // Check if EXTI line interrupt was detected
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET)  {
    // Clear the interrupt (has to be done for EXTI)
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
    // Toggle LED
    HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14);
		
		BaseType_t higherPriorityTaskWoken;
		higherPriorityTaskWoken = pdTRUE;
    xSemaphoreGiveFromISR(mySemaphore, &higherPriorityTaskWoken);
		//portYIELD_FROM_ISR(higherPriorityTaskWoken);
  }
}

/** Writes string to USART using polling. */
void USART_POLL_WriteString(const char *string)
{
  size_t i=0;
  // for each character in the given string
  while (string[i]) {
    // write the current character to USART's data register (DR)
    USART1->DR = string[i];
    // wait until status register (SR) indicates that the transmitter is empty again (TXE)
    while((USART1->SR & UART_FLAG_TXE) == 0) {
      ;
    }
    // move to the next character
    i++;
  }
}

void taskHELLO(void* params)
{
	  static SemaphoreHandle_t myMutex;
	
		taskENTER_CRITICAL();
	
		if(myMutex == NULL){
			myMutex = xSemaphoreCreateMutex();//mutex
		}
		
		taskEXIT_CRITICAL();
			
		while (1) {
			if( xSemaphoreTake(myMutex, 100)==pdTRUE){//mutex
				USART_POLL_WriteString("hello world\r\n");
				xSemaphoreGive(myMutex);////mutex
				
			}
  }
}

//----------------------Queue--------------------------------------------
QueueHandle_t myQueue;

void taskTXQueue(void* params){
	int element = 1;
	while(1){
		if (xQueueSend(myQueue, &element, 1000) == pdPASS) {
			element++;
		}
	}
}

void taskRXQueue(void* params){
	int element;
	char data[10];
	
	while(1){
		if (xQueueReceive(myQueue, &element, 1000) == pdTRUE ) {
			// element was received successfully
			sprintf(data, "%d\r\n", element);
			USART_POLL_WriteString(data);
			
		} else {
		// unable to receive elements from the queue in the given time = 1000 ticks
		}
	}
}

//Function to increase frequency accuracy
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();
    /* The voltage scaling allows optimizing the power consumption when the
    device is clocked below the maximum system frequency (see datasheet). */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
    RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

/**
 * Application entry point
 */
//int main(void)
//{
//    // Initialize STM32Cube HAL library
//    HAL_Init();

//		//System Clock Config
//		SystemClock_Config();
//	
//		//USART Init
//		USART_Init();
//	
//		//Both LED Init
//		LED_Init();
//	
//		//Trace init
//		TRACE_Init();
//	
//		//mySemaphore = xSemaphoreCreateBinary(); 
//		//myMutex = xSemaphoreCreateMutex();
//	
//		EXTI2_Init();
//	
////		static BlinkParams bp1 = { .gpio = GPIOG, .pin = GPIO_PIN_13, .ticks = 500};
////		static BlinkParams bp2 = { .gpio = GPIOG, .pin = GPIO_PIN_14, .ticks = 1000};  
//		
////		TaskHandle_t taskHandle_led1;
////		TaskHandle_t taskHandle_led2;
////		TaskHandle_t taskHandle_echo;
//				
////		if (pdPASS != xTaskCreate(taskLED, "led1", configMINIMAL_STACK_SIZE, &bp1, 3, &taskHandle_led1)) {
////			printf("ERROR: Unable to create task!\n");
////		}
////		if (pdPASS != xTaskCreate(taskLED, "led2", configMINIMAL_STACK_SIZE, &bp2, 3, &taskHandle_led2)) {
////			printf("ERROR: Unable to create task!\n");
////		}
////	
////		if (pdPASS != xTaskCreate(taskUSART, "echo", configMINIMAL_STACK_SIZE, NULL, 3, &taskHandle_echo)) {
////			printf("ERROR: Unable to create task!\n");
////		}
//	

////SEMAPHORE
////		if (pdPASS != xTaskCreate(taskLED_semaphore, "led1_Semaphore", configMINIMAL_STACK_SIZE, NULL, 3, NULL)) {
////			printf("ERROR: Unable to create task!\n");
////		}


////Create 2 HELLO tasks - MUTEX
////			TaskHandle_t taskHandle;
////			if (pdPASS != xTaskCreate(taskHELLO, "hell1", configMINIMAL_STACK_SIZE, NULL, 3, &taskHandle)) {
////				printf("ERROR: Unable to create task!\n");
////			}
////			TaskHandle_t taskHandle1;
////			if (pdPASS != xTaskCreate(taskHELLO, "hell2", configMINIMAL_STACK_SIZE, NULL, 3, &taskHandle1)) {
////				printf("ERROR: Unable to create task!\n");
////			}
//		
////QUEUE

//		//TaskHandle_t taskHandle;
//		//TaskHandle_t taskHandle1;
//		
//		myQueue = xQueueCreate(10, sizeof(int));
//		
//		if (pdPASS != xTaskCreate(taskTXQueue, "taskTXQueue", configMINIMAL_STACK_SIZE, NULL, 3, NULL)) {
//			printf("ERROR: Unable to create task!\n");
//		}
//		
//		if (pdPASS != xTaskCreate(taskRXQueue, "taskRXQueue", configMINIMAL_STACK_SIZE, NULL, 3, NULL)) {
//			printf("ERROR: Unable to create task!\n");
//		}
//		
//		
//		//TRACE_BindTaskWithTrace(taskHandle_led1, 1);
//		//TRACE_BindTaskWithTrace(taskHandle_led2, 2);
//		//TRACE_BindTaskWithTrace(taskHandle_echo, 3);
//		
//		//Schedule tasks
//    vTaskStartScheduler();
//	
//} /* main */

xQueueHandle myQueue;
xSemaphoreHandle myMutex;

int main (void)
{
    // Initialize STM32Cube HAL library
    HAL_Init();
    // Initialize LED pins
    LED_Init();
    // Create mutex
    myMutex= xSemaphoreCreateMutex();
    // Create queue
    myQueue= xQueueCreate(100, 1);
    // Create RTOS tasks
    if (pdPASS != xTaskCreate(taskLED, "led", 200, NULL, 3, NULL )) {
    // should never get here, unless there is a memory allocation problem
    }
    // start FreeRTOS scheduler - the RTOS takes control over the microcontroller
    vTaskStartScheduler();
} /* main */