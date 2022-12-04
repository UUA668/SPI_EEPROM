/*
 * M95.EEPROM.c
 *
 *  Created on: 24.11.2022
 *      Author: markfe1
 */

#include "M95_EEPROM.h"

static uint8_t Instruction_WREN = 0b00000110;	/*Write enable*/
static uint8_t Instruction_WRDI = 0b00000100;	/*Write disable*/
static uint8_t Instruction_RDSR = 0b00000101;	/*Read status register*/
static uint8_t Instruction_WRSR = 0b00000001;	/*Write status register*/
static uint8_t Instruction_READ_LOW = 0b00000011;	/* Read from memory up to address 255*/
static uint8_t Instruction_WRITE_LOW = 0b00000010;	/* Write to memory up to address 255*/
static uint8_t Instruction_READ_HIGH = 0b00001011;	/* Read from memory above address 255*/
static uint8_t Instruction_WRITE_HIGH = 0b00001010;	/* Write to memory above address 255*/
static uint16_t Start_Address = 0u;
static uint8_t Overwrite = 0xFF;				/* overwrite value*/
static uint8_t Buffer_RDSR;


M95_status_t M95_Init(SPI_HandleTypeDef *hspi)
{
	/*check the incoming parameters*/
		if(NULL == hspi)
		{
			return M95_NOK;
		}

	/*enable WEL*/


		/*HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);		/*U5 chip selected */
		/*HAL_SPI_Transmit(hspi, &Instruction_WREN, 1, 100);			/*send Write Enable*/
		/*HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */



		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);		/*U5 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, &Buffer_RDSR, 1, 100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */

		if(Buffer_RDSR != NULL)
		{
			return M95_OK;

		}

		return M95_NOK;
}

M95_status_t M95_Read(SPI_HandleTypeDef *hspi,uint8_t *pData)
{

		int i = 0;
		do{
				pData[i] = 0u;
				i++;
			} while(i < EEPROM_SIZE);

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);		/*U5 chip select */
		HAL_SPI_Transmit(hspi, &Instruction_READ_LOW, 1, 100);
		HAL_SPI_Receive(hspi, pData, 512, 1000);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */

		return M95_OK;
}

M95_status_t M95_Clear(SPI_HandleTypeDef *hspi, uint8_t *pData)
{
	int i = 0;
	do{
			pData[i] = Overwrite;
			i++;
		} while(i < PAGE_SIZE);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);			/*disable Write Protection */

	do{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);			/*U5 chip select */
			HAL_SPI_Transmit(hspi, &Instruction_WREN, 1, 100);			/*send Write Enable*/
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);		/*U5 chip select */
			if(Start_Address > 256)
				{
				HAL_SPI_Transmit(hspi, &Instruction_WRITE_HIGH, 1, 100);
				}
			else
				{
				HAL_SPI_Transmit(hspi, &Instruction_WRITE_LOW, 1, 100);
				}
			HAL_SPI_Transmit(hspi, &Start_Address, 1, 100);
			HAL_SPI_Transmit(hspi, pData, PAGE_SIZE, 1000);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */
			Start_Address = Start_Address+PAGE_SIZE;					/*setup the new write address*/
			M95_Wait_Until_WIP(hspi);

	}while(Start_Address <= EEPROM_SIZE);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);		/*enable Write Protection */

			return M95_OK;
}

M95_status_t M95_Wait_Until_WIP(SPI_HandleTypeDef *hspi)
{
	uint8_t wip = 1;
	uint8_t mask = 1;
	do
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);		/*U5 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, &Buffer_RDSR, 1, 100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */

		wip = Buffer_RDSR & mask;								/*mask wip bit*/

	} while(wip > 0);
	return M95_OK;
}

