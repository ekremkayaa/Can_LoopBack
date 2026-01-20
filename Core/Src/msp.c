/*
 * msp.c
 *
 *  Created on: Jan 13, 2026
 *      Author: ekrem
 */

#include "main.h"

void HAL_MspInit(void){
	//here will do low level processor specific inits
	//1. set up priority grouping of the arm cortex mx processor
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	//2. enable required system exceptions of the arm cortex mx processor
	SCB->SHCSR |= 0x7 << 16; //usage fault, memory fault and bus fault system exceptions

	//3. configure the priority for the system exceptions
	HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
	HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
	HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
	//HAL_Init handle the systick
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart){
	GPIO_InitTypeDef gpio_uart;
	//here we are going to do the low level inits of the usart2 peripherals

	//1. enable the clock for the USART2 and GPIOA peripheral
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	//2. do the pin muxing congigurations
	gpio_uart.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	gpio_uart.Mode = GPIO_MODE_AF_PP;
	gpio_uart.Pull = GPIO_PULLUP;
	gpio_uart.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_uart.Alternate = GPIO_AF7_USART2;  //UART2_TX
	HAL_GPIO_Init(GPIOA, &gpio_uart);

	//3. enable the irq and set up the priority (NVIC settings)
	/*
	 * Now, first of all, we have understand why it is required. You know that peripheral has the capacity to generate
	 * asynchronous events that we call as interrupts. So, in the case of UART peripherals, for example, what are the
	 * asynchronous events? The asynchronous events can be completion of transmission or completion of reception or
	 * detection of any error, like that. So, all these are asynchronous events, right? For example, you can make CPU to go to
	 * sleep until the transmission completes. So, when the transmission completes UART peripheral can generate interrupt,
	 * which can make up the processor. So, we can save some power, isn't it. Because until the transmission or reception
	 * finishes the processor or CPU can go to sleep. . So, the interrupt mode will be very helpful in the case of low power
	 * applications as well.
	 */
	HAL_NVIC_EnableIRQ(USART2_IRQn); //stm32f4xx_hal_cortex.h
	HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan){
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}
