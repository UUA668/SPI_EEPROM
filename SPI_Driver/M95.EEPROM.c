/*
 * M95.EEPROM.c
 *
 *  Created on: 24.11.2022
 *      Author: markfe1
 */

#include "M95_EEPROM.h"

static uint8_t *Instruction_WREN = 0b00000110;
static uint8_t *Instruction_RDSR = 0b00000101;




M95_status_t M95_Init(SPI_HandleTypeDef *hspi, uint8_t *pData)
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
		HAL_SPI_Receive(hspi, pData, 1, 100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */


}

M95_status_t M95_Read(SPI_HandleTypeDef *hspi,uint8_t *pData)
{
	/*U5 chip select */
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);


		if(HAL_SPI_Receive(hspi, *pData, 512, 100) == HAL_OK)
		{
			/*U5 chip deselect */
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);

						return M95_OK;
		}
		/*U5 chip deselect */
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);

		return M95_NOK;
}




