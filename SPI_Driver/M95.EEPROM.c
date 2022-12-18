/*
 * M95.EEPROM.c
 *
 *  Created on: 24.11.2022
 *      Author: markfe1
 */

#include "M95_EEPROM.h"

static uint8_t Instruction_WREN = 0b00000110;		/*Write enable*/
static uint8_t Instruction_WRDI = 0b00000100;		/*Write disable*/
static uint8_t Instruction_RDSR = 0b00000101;		/*Read status register*/
static uint8_t Instruction_WRSR = 0b00000001;		/*Write status register*/
static uint8_t Instruction_READ = 0b00000011;		/* Read from memory*/
static uint8_t Instruction_READ_High = 0b00001011;	/* Read from memory above address 255 by U5*/
static uint8_t Instruction_WRITE = 0b00000010;		/* Write to memory*/
static uint8_t Instruction_WRITE_High = 0b00001010;	/* Write to memory above address 255 by U5*/
static uint8_t Overwrite = 0x19;					/* overwrite value*/
static uint8_t Buffer_RDSR;

EEPROM_Config_t EEPROM_Dev_List[] =
{
		{0x50000000 + 0x000 + 0x14, 10, 16, 512},			/*M95040-RMC6TG, U5 on the board*/
		{0x50000000 + 0x800 + 0x14, 7, 64, 32000},			/*M95256-DFDW6TP, U6 on the board*/
		{0x50000000 + 0x400 + 0x14, 0, 512, 512000},		/*M95M04-DRMN6TP, U7 on the board*/
};


M95_status_t M95_Init(SPI_HandleTypeDef *hspi)
{
	uint8_t RDSR_U5 =0;
	uint8_t RDSR_U6 =0;
	uint8_t RDSR_U7 =0;

	/*check the incoming parameters*/
		if(NULL == hspi)
		{
			return M95_NOK;
		}


		*((uint32_t*)U5.CSPort) = (0<<U5.CSPin); 						/*U5 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, &RDSR_U5, 1, 100);
		*((uint32_t*)U5.CSPort) = (1<<U5.CSPin);						/*U5 chip deselect */

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);				/*disable Write Protection */
		*((uint32_t*)U6.CSPort) = (0<<U6.CSPin); 						/*U6 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_WREN, 1, 100);
		*((uint32_t*)U6.CSPort) = (1<<U6.CSPin);						/*U6 chip deselect */
		*((uint32_t*)U6.CSPort) = (0<<U6.CSPin); 						/*U6 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_WRSR, 1, 100);				/*Write Status Register*/
		RDSR_U6 = 0x02;													/*disable Write Protect in Status Register*/
		HAL_SPI_Transmit(hspi, &RDSR_U6, 1, 100);
		*((uint32_t*)U6.CSPort) = (1<<U6.CSPin);						/*U6 chip deselect */
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);			/*enable Write Protection */

		RDSR_U6 =0;
		*((uint32_t*)U6.CSPort) = (0<<U6.CSPin); 						/*U6 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, &RDSR_U6, 1, 100);
		*((uint32_t*)U6.CSPort) = (1<<U6.CSPin);						/*U6 chip deselect */

		*((uint32_t*)U7.CSPort) = (0<<U7.CSPin); 						/*U7 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, &RDSR_U7, 1, 100);
		*((uint32_t*)U7.CSPort) = (1<<U7.CSPin);						/*U7 chip deselect */


		if((0xF0 == RDSR_U5) && (0x00 == RDSR_U6) && (0x00 == RDSR_U7))
		{
			return M95_OK;

		}

		return M95_NOK;
}

M95_status_t M95_Read(SPI_HandleTypeDef *hspi, EEPROM_Config_t *pEEPROM, uint32_t Start_Address, uint16_t Read_Size, uint8_t *pData)
{

		int i = 0;
		do{
				pData[i] = 0u;
				i++;
			} while(i < READ_BUFFER_SIZE);

		*((uint32_t*)pEEPROM->CSPort) = (0<<pEEPROM->CSPin); 						/*chip selected */

/*---------------------------------Instruction----------------------------------*/
		if((pEEPROM->MemSize <= 512) && (Start_Address > 255))
		{
			HAL_SPI_Transmit(hspi, &Instruction_READ_High, 1, 100);					/*set A8 Address bit for U5*/
		}
		else
		{
			HAL_SPI_Transmit(hspi, &Instruction_READ, 1, 100);
		}

/*-----------------------------------Address------------------------------------*/
		if (pEEPROM->MemSize > 64000)												/*128kbyte and above EEPROMs are 3byte Address*/
		{
			HAL_SPI_Transmit(hspi, ((uint8_t *) &Start_Address)+2, 1, 100);			/*send the the highest address byte (24...16bit)*/
			HAL_SPI_Transmit(hspi, ((uint8_t *) &Start_Address)+1, 1, 100);			/*send the the highest address byte (15...8bit)*/
		}
		else if ((pEEPROM->MemSize > 512) && (pEEPROM->MemSize <= 64000))			/*between 1kbyte and 64kbyte EEPROMs are 2byte Address*/
		{
		HAL_SPI_Transmit(hspi, ((uint8_t *) &Start_Address)+1, 1, 100);				/*send the the highest address byte (15...8bit)*/
		}
		HAL_SPI_Transmit(hspi, ((uint8_t *) &Start_Address), 1, 100);

/*----------------------------------Read Data----------------------------------*/
		HAL_SPI_Receive(hspi, pData, Read_Size, 1000);
		*((uint32_t*)pEEPROM->CSPort) = (1<<pEEPROM->CSPin);						/*chip deselect */

		return M95_OK;
}

M95_status_t M95_Write(SPI_HandleTypeDef *hspi, EEPROM_Config_t *pEEPROM, uint32_t Start_Address, uint16_t Write_Size, uint8_t *pData)
{

		/*if((Start_Address+Write_Size) > pEEPROM->MemSize)
		{
			return M95_NOK;
		}
*/


		int i = 0;
		do{
			pData[i] = Overwrite;
			i++;
		} while(i < pEEPROM->PageSize);


		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);							/*disable Write Protection */

		*((uint32_t*)pEEPROM->CSPort) = (0<<pEEPROM->CSPin); 						/*chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_WREN, 1, 100);							/*send Write Enable*/
		*((uint32_t*)pEEPROM->CSPort) = (1<<pEEPROM->CSPin);						/*chip deselect */

		*((uint32_t*)pEEPROM->CSPort) = (0<<pEEPROM->CSPin); 						/*chip selected */

/*---------------------------------Instruction----------------------------------*/
		if((pEEPROM->MemSize <= 512) && (Start_Address > 255))
		{
			HAL_SPI_Transmit(hspi, &Instruction_WRITE_High, 1, 100);				/*set A8 Address bit for U5*/
		}
		else
		{
			HAL_SPI_Transmit(hspi, &Instruction_WRITE, 1, 100);
		}

/*-----------------------------------Address------------------------------------*/
		if (pEEPROM->MemSize > 64000)												/*128kbyte and above EEPROMs are 3byte Address*/
		{
			HAL_SPI_Transmit(hspi, ((uint8_t *) &Start_Address)+2, 1, 100);			/*send the the highest address byte (24...16bit)*/
			HAL_SPI_Transmit(hspi, ((uint8_t *) &Start_Address)+1, 1, 100);			/*send the the highest address byte (15...8bit)*/
		}
		else if ((pEEPROM->MemSize > 512) && (pEEPROM->MemSize <= 64000))				/*between 1kbyte and 64kbyte EEPROMs are 2byte Address*/
		{
			HAL_SPI_Transmit(hspi, ((uint8_t *) &Start_Address)+1, 1, 100);			/*send the the highest address byte (15...8bit)*/
		}
		HAL_SPI_Transmit(hspi, ((uint8_t *) &Start_Address), 1, 100);

/*--------------------------------Write Data------------------------------------*/
		HAL_SPI_Transmit(hspi, pData, PAGE_SIZE, 1000);
		*((uint32_t*)pEEPROM->CSPort) = (1<<pEEPROM->CSPin);						/*U5 chip deselect */

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);						/*enable Write Protection */

		return M95_OK;
}



M95_status_t M95_Clear(SPI_HandleTypeDef *hspi, uint8_t *pData)
{
//	int i = 0;
//	do{
//			pData[i] = Overwrite;
//			i++;
//		} while(i < PAGE_SIZE);
//
//			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);			/*disable Write Protection */
//
//	do{
//			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);		/*U5 chip select */
//			HAL_SPI_Transmit(hspi, &Instruction_WREN, 1, 100);			/*send Write Enable*/
//			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */
//			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);		/*U5 chip select */
//			if(Start_Address > sizeof(uint8_t))
//				{
//				HAL_SPI_Transmit(hspi, &Instruction_WRITE_HIGH, 1, 100);
//				}
//			else
//				{
//				HAL_SPI_Transmit(hspi, &Instruction_WRITE_LOW, 1, 100);
//				}
//			HAL_SPI_Transmit(hspi, &Start_Address, 1, 100);
//			HAL_SPI_Transmit(hspi, pData, PAGE_SIZE, 1000);
//			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);		/*U5 chip deselect */
//			Start_Address = Start_Address+PAGE_SIZE;					/*setup the new write address*/
//			M95_Wait_Until_WIP(hspi);
//
//	}while(Start_Address <= EEPROM_SIZE);
//
//			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);		/*enable Write Protection */

			return M95_OK;
}

M95_status_t M95_Wait_Until_WIP(SPI_HandleTypeDef *hspi)
{
	uint8_t wip = 1;
	uint8_t mask = 1;
	do
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);			/*U5 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, &Buffer_RDSR, 1, 100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);			/*U5 chip deselect */

		wip = Buffer_RDSR & mask;										/*mask wip bit*/

	} while(wip > 0);
	return M95_OK;
}

