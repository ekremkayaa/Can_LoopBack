/*
 * it.c
 *
 *  Created on: Jan 13, 2026
 *      Author: ekrem
 */

#include "main.h"

extern UART_HandleTypeDef huart2;
/*
 * STM32 Cube framework project, remember that Systick timer will be ticking in the background for every 1ms.
 * So there will be Systick interrupt for every 1ms. And the entire STM32 Cube layer depends upon that.
 * All the time of related functionalities and other timekeeping functionalities of the STM32 Cube layer is dependent on that
 * Systick timer which is ticking for every 1ms, right? So, that's why we have to now implement the systick handler in our code
 * because we don't have it, isn't it? . And we have to increment the global tick variable inside the systick handler.
 * Because, the cube layer is depending on that global tick variable value.
 */

void SysTick_Handler(void){
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

/*
 * interrupt can be triggered for so many reasons. So, we don't know why exactly UART triggered the interrupt.
 * It may be due to completion of transmission, it may be due to completion of reception, it may be due to error.
 * We exactly don't know why the interrupt happened. So that's why, once the IRQ handler is called, you need to process
 * that interrupt. For that, you have to call interrupt processing API given in the driver file.
 */
void USART2_IRQHandler(void){
	/*So, that also means that this function actually processes the received interrupt. So that's why, so,
	 * the purpose of this function is to identify the reason for that interrupt. So it goes through various
	 * status registers of the UART to identify the cause for an interrupt.
	 */
	HAL_UART_IRQHandler(&huart2);
	/*
	 * if you just go and analyze this interrupt processing API, let's go inside that. And here you will see that it goes
	 * through lots of checks and at the end, if you just go here you can see something like callbacks. So, if this interrupt
	 * is because of error then you will get the error callback. If this interrupt is because of reception of data bytes then
	 * you will get a callback called receive complete callback. And if this interrupt is because of transmission complete
	 * then you will get transmission complete callback. That means this interrupt processing API is able to provide or
	 * call lots of callback functions which must be implemented in the user application.
	 */
}

