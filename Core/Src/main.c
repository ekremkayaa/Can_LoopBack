/*
 * main.c
 *
 *  Created on: Jan 13, 2026
 *      Author: ekrem
 */
#include "main.h"
#include <stdio.h>
#include <string.h>

void UART2Init(void);
void systemClockConfigHSE(uint8_t clock_freq);
void errorHandler(void);
void CAN1Init(void);
void CAN1Tx(void);
void CAN1Rx(void);
void CANFilterConfig(void);

UART_HandleTypeDef huart2;
CAN_HandleTypeDef hcan1;

int main(){

	HAL_Init();

	systemClockConfigHSE(SYS_CLOCK_FREQ_50_MHZ);

	UART2Init();

	//put can initialization mode
	CAN1Init();
	//and leaves initialization mode

	CANFilterConfig();

	if( HAL_CAN_Start(&hcan1) != HAL_OK){
		errorHandler();
	}

	CAN1Tx();

	CAN1Rx();

	while(1);

	return 0;
}

void UART2Init(void){
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;

	if(HAL_UART_Init(&huart2) != HAL_OK){
		errorHandler();
	}
}

void CAN1Init(void){
	hcan1.Instance = CAN1;
	hcan1.Init.Mode = CAN_MODE_LOOPBACK;
	hcan1.Init.AutoBusOff = DISABLE;
	hcan1.Init.AutoRetransmission = ENABLE;
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE; //RFLM register
	hcan1.Init.TimeTriggeredMode = DISABLE;
	/*
	 * 0: Receive FIFO not locked on overrun. Once a receive FIFO is full the next incoming message overwrites the previous one.
	 * 1: Receive FIFO locked against overrun. Once a receive FIFO is full the next incoming  message is discarded.
	 */
	hcan1.Init.TransmitFifoPriority = DISABLE;

	//settings related to can bit timings
	hcan1.Init.Prescaler = 5;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan1.Init.TimeSeg1 = CAN_BS1_8TQ;
	hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;

	if(HAL_CAN_Init(&hcan1) != HAL_OK){
		errorHandler();
	}
}

void CAN1Tx(void){
	CAN_TxHeaderTypeDef TxHeader;

	uint32_t TxMailbox;

	char msg[50];
	uint8_t our_message[5] = {'H','E','L','L','O'};

	TxHeader.DLC = 5;
	TxHeader.StdId = 0x65D;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;

	//request transmitting
	if( HAL_CAN_AddTxMessage(&hcan1, &TxHeader, our_message, &TxMailbox)!= HAL_OK){
		errorHandler();
	}
	//wait until pending empty
	while(HAL_CAN_IsTxMessagePending(&hcan1, TxMailbox));

	sprintf(msg,"Message Transmitted\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void CAN1Rx(void){
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t rcvd_msg[5];
	char msg[50];

	//we are waiting for at least one message in to the RX FIFO0, Number of messages available in Rx FIFO
	while(!HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0));

	//polling mode
	if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, rcvd_msg) != HAL_OK){
		errorHandler();
	}

	sprintf(msg,"Message Received : %s\r\n",rcvd_msg);
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void CANFilterConfig(void){
	CAN_FilterTypeDef can1_filter_init;

	can1_filter_init.FilterActivation = ENABLE;
	can1_filter_init.FilterBank = 0;
	can1_filter_init.FilterFIFOAssignment = CAN_RX_FIFO0;
	can1_filter_init.FilterIdHigh = 0x0000;
	can1_filter_init.FilterIdLow = 0x0000;
	can1_filter_init.FilterMaskIdHigh = 0x0000;
	can1_filter_init.FilterMaskIdLow = 0x0000;
	can1_filter_init.FilterMode = CAN_FILTERMODE_IDMASK;
	can1_filter_init.FilterScale = CAN_FILTERSCALE_32BIT;

	if (HAL_CAN_ConfigFilter(&hcan1, &can1_filter_init) != HAL_OK){
		errorHandler();
	}
}

void systemClockConfigHSE(uint8_t clock_freq){
	RCC_OscInitTypeDef osc_init;
	RCC_ClkInitTypeDef clk_init;
	uint8_t FLatency = 0;

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc_init.HSEState = RCC_HSE_ON;
	//osc_init.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT; //no need calibration for hse
	osc_init.PLL.PLLState = RCC_PLL_ON;
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	switch(clock_freq){
		   case SYS_CLOCK_FREQ_50_MHZ:{
			osc_init.PLL.PLLM = 8;
			osc_init.PLL.PLLN = 100;
			osc_init.PLL.PLLP = 2;
			osc_init.PLL.PLLQ = 2; //default
			//osc_init.PLL.PLLR = 2; //default

			clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
								 RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
			clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
			clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

			FLatency = FLASH_ACR_LATENCY_1WS; // rm0090 Relation between CPU clock frequency and flash memory read time
			break;
		  }
		  case SYS_CLOCK_FREQ_84_MHZ: {
			osc_init.PLL.PLLM = 8;
			osc_init.PLL.PLLN = 168;
			osc_init.PLL.PLLP = 2;
			osc_init.PLL.PLLQ  = 2;
			//osc_init.PLL.PLLR=2;

			clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
								 RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
			clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
			clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

			FLatency = FLASH_ACR_LATENCY_2WS;

			break;
		  }
		  case SYS_CLOCK_FREQ_120_MHZ: {
			osc_init.PLL.PLLM = 8;
			osc_init.PLL.PLLN = 240;
			osc_init.PLL.PLLP = 2;
			osc_init.PLL.PLLQ  = 2;
			//osc_init.PLL.PLLR=2;

			clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
								 RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
			clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			clk_init.APB1CLKDivider = RCC_HCLK_DIV4;
			clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

			FLatency = FLASH_ACR_LATENCY_3WS;
			break;
		  }
		  case SYS_CLOCK_FREQ_168_MHZ: {
			//enable the clock for the power controller
			__HAL_RCC_PWR_CLK_ENABLE();

			//set regulator voltage scale as 1
			__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

			//turn on the over drive mode of voltage regulator for nucleo
			//__HAL_PWR_OVERDRIVE_ENABLE();

			osc_init.PLL.PLLM = 4;
			osc_init.PLL.PLLN = 168;
			osc_init.PLL.PLLP = 2;
			osc_init.PLL.PLLQ  = 2;
			//osc_init.PLL.PLLR=2;

			clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
								 RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
			clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			clk_init.APB1CLKDivider = RCC_HCLK_DIV4;
			clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

			FLatency = FLASH_ACR_LATENCY_7WS;
			break;
		  }

		  default:
			return;
	  }

	  if(HAL_RCC_OscConfig(&osc_init) != HAL_OK){
		  errorHandler();
	  }

	  if(HAL_RCC_ClockConfig(&clk_init,FLatency) != HAL_OK){
		  errorHandler();
	  }

	  //systick conf
	  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
	  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void errorHandler(void){
	while(1);
}
