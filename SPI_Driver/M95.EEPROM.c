/*
 * M95.EEPROM.c
 *
 *  Created on: 24.11.2022
 *      Author: markfe1
 */

#include "M95_EEPROM.h"
#include "stm32g071xx.h"
#include "stm32g0xx_hal_gpio.h"

static uint8_t Instruction_WREN = 0b00000110;		/*Write enable*/
static uint8_t Instruction_WRDI = 0b00000100;		/*Write disable*/
static uint8_t Instruction_RDSR = 0b00000101;		/*Read status register*/
static uint8_t Instruction_WRSR = 0b00000001;		/*Write status register*/
static uint8_t Instruction_READ = 0b00000011;		/* Read from memory*/
static uint8_t Instruction_READ_High = 0b00001011;	/* Read from memory above address 255 by U5*/
static uint8_t Instruction_WRITE = 0b00000010;		/* Write to memory*/
static uint8_t Instruction_WRITE_High = 0b00001010;	/* Write to memory above address 255 by U5*/
static uint8_t Overwrite = 0x24;					/* overwrite value*/
static uint8_t ClearValue = 0x00;					/* clear value*/
static uint8_t Buffer_RDSR;

EEPROM_Config_t EEPROM_Dev_List[] =
{
		{0x50000000 + 0x000, 0x400, 16, 512},		/*M95040-RMC6TG, U5 on the board*/
		{0x50000000 + 0x800, 0x80, 64, 32000},			/*M95256-DFDW6TP, U6 on the board*/
		{0x50000000 + 0x400, 0x1, 512, 512000},		/*M95M04-DRMN6TP, U7 on the board*/
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


		HAL_GPIO_WritePin(U5.CSPort, U5.CSPin, GPIO_PIN_RESET);			/*U5 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, &RDSR_U5, 1, 100);
		HAL_GPIO_WritePin(U5.CSPort, U5.CSPin, GPIO_PIN_SET);			/*U5 chip deselect */

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);				/*disable Write Protection */
		HAL_GPIO_WritePin(U6.CSPort, U6.CSPin, GPIO_PIN_RESET);	 		/*U6 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_WREN, 1, 100);
		HAL_GPIO_WritePin(U6.CSPort, U6.CSPin, GPIO_PIN_SET);			/*U6 chip deselect */
		HAL_GPIO_WritePin(U6.CSPort, U6.CSPin, GPIO_PIN_RESET);	 		/*U6 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_WRSR, 1, 100);				/*Write Status Register*/
		RDSR_U6 = 0x02;													/*disable Write Protect in Status Register*/
		HAL_SPI_Transmit(hspi, &RDSR_U6, 1, 100);
		HAL_GPIO_WritePin(U6.CSPort, U6.CSPin, GPIO_PIN_SET);			/*U6 chip deselect */
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);			/*enable Write Protection */

		RDSR_U6 =0;
		HAL_GPIO_WritePin(U6.CSPort, U6.CSPin, GPIO_PIN_RESET);	 		/*U6 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, &RDSR_U6, 1, 100);
		HAL_GPIO_WritePin(U6.CSPort, U6.CSPin, GPIO_PIN_SET);			/*U6 chip deselect */

		HAL_GPIO_WritePin(U7.CSPort, U7.CSPin, GPIO_PIN_RESET);	 		/*U7 chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, &RDSR_U7, 1, 100);
		HAL_GPIO_WritePin(U7.CSPort, U7.CSPin, GPIO_PIN_SET);			/*U7 chip deselect */


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
		HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_RESET); 		/*chip selected */

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
		HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_SET);			/*chip deselect */

		return M95_OK;
}

M95_status_t M95_Write(SPI_HandleTypeDef *hspi, EEPROM_Config_t *pEEPROM, uint32_t Start_Address, uint16_t Write_Size, uint8_t *pData)
{

		if((Start_Address+Write_Size) > pEEPROM->MemSize)
		{
			return M95_NOK;
		}

		uint32_t OverPage = 0;

		OverPage = Start_Address;

		while (OverPage > pEEPROM->PageSize){
			OverPage -= pEEPROM->PageSize;
		}

		int i = 0;
		do{
			pData[i] = Overwrite;
			i++;
		} while(i < Write_Size);


		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);							/*disable Write Protection */

		HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_RESET); 		/*chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_WREN, 1, 100);							/*send Write Enable*/
		HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_SET);			/*chip deselect */

		HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_RESET); 		/*chip selected */

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
		HAL_SPI_Transmit(hspi, pData, (Write_Size - OverPage), 1000);
		HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_SET);			/*chip deselect */

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);						/*enable Write Protection */

		if (M95_Wait_Until_WIP(hspi, pEEPROM, 100) != M95_OK)
		{
			return M95_NOK;
		}

		return M95_OK;
}



M95_status_t M95_Clear(SPI_HandleTypeDef *hspi, EEPROM_Config_t *pEEPROM)
{
	uint8_t Clear_Data [pEEPROM->PageSize];
	uint32_t Start_Address = 0;

	int i = 0;
	do{
			Clear_Data[i] = ClearValue;
			i++;
		} while(i < pEEPROM->PageSize);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);			/*disable Write Protection */

	do{
			HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_RESET); 		/*chip selected */
			HAL_SPI_Transmit(hspi, &Instruction_WREN, 1, 100);							/*send Write Enable*/
			HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_SET);			/*chip deselect */

			HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_RESET); 		/*chip selected */

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
			else if ((pEEPROM->MemSize > 512) && (pEEPROM->MemSize <= 64000))			/*between 1kbyte and 64kbyte EEPROMs are 2byte Address*/
			{
				HAL_SPI_Transmit(hspi, ((uint8_t *) &Start_Address)+1, 1, 100);			/*send the the highest address byte (15...8bit)*/
			}
			HAL_SPI_Transmit(hspi, ((uint8_t *) &Start_Address), 1, 100);

		/*--------------------------------Write Data------------------------------------*/
			HAL_SPI_Transmit(hspi, &Clear_Data[0], pEEPROM->PageSize, 1000);
			HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_SET);			/*chip deselect */

			Start_Address = Start_Address+(pEEPROM->PageSize);							/*setup the new write address*/
			M95_Wait_Until_WIP(hspi, pEEPROM, 1000);

	}while(Start_Address <= pEEPROM->MemSize);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);		/*enable Write Protection */

			return M95_OK;
}

M95_status_t M95_Wait_Until_WIP(SPI_HandleTypeDef *hspi, EEPROM_Config_t *pEEPROM, uint32_t Timeout)
{
	uint32_t tickstart;
	uint8_t wip = 1;
	uint8_t mask = 1;

	tickstart = HAL_GetTick();

	do
	{
		HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_RESET); 	/*chip selected */
		HAL_SPI_Transmit(hspi, &Instruction_RDSR, 1, 100);
		HAL_SPI_Receive(hspi, &Buffer_RDSR, 1, 100);
		HAL_GPIO_WritePin(pEEPROM->CSPort, pEEPROM->CSPin, GPIO_PIN_SET); 		/*chip deselected */

		wip = Buffer_RDSR & mask;												/*mask wip bit*/

		if((HAL_GetTick() - tickstart) >=  Timeout)
		{
			return M95_NOK;
		}

	} while(wip > 0);
	return M95_OK;
}

