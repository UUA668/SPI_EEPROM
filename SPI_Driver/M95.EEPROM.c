/*
 * M95.EEPROM.c
 *
 *  Created on: 24.11.2022
 *      Author: markfe1
 */

#include "M95_EEPROM.h"

static uint8_t *Instruction_WREN = 0b00000110;
static uint8_t *Instruction_RDSR = 0b00000101;
static uint8_t *Instruction_READ = 0b00000011;	/* warning! A8 address bit not used*/



M95_status_t M95_Init(SPI_HandleTypeDef *hspi, uint8_t *pBuffer_RDSR)
{
	/*check the incoming parameters*/
		if(NULL == hspi)
		{
			return M95_NOK;
		}

	/*enable WEL*/


		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);		/*U5 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_WREN, 1, 100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */



		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);		/*U5 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, pBuffer_RDSR, 1, 100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */

		if(*pBuffer_RDSR != NULL)
		{
			return M95_OK;

		}

		return M95_NOK;
}

M95_status_t M95_Read(SPI_HandleTypeDef *hspi,uint8_t *pData)
{
	/*U5 chip select */
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);

		HAL_SPI_Transmit(hspi, &Instruction_READ, 1, 100);
		HAL_SPI_Receive(hspi, *pData, 512, 100);


	/*U5 chip deselect */
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);

		return M95_OK;
}




