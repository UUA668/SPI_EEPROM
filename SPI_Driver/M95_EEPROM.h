/*
 * M95_EEPROM.h
 *
 *  Created on: 24.11.2022
 *      Author: markfe1
 */

#ifndef M95_EEPROM_H_
#define M95_EEPROM_H_

#include "stm32g0xx_hal.h"

#define READ_BUFFER_SIZE 512
#define PAGE_SIZE 16
#define U5 EEPROM_Dev_List[0]
#define U6 EEPROM_Dev_List[1]
#define U7 EEPROM_Dev_List[2]

/*M95 Status definicio*/
typedef enum {
	M95_OK = 0,
	M95_NOK,
} M95_status_t;

/*EEPROM communication structure*/
typedef struct{
	uint32_t 			CSPort;			/*GPIO PORT Address for Device Chip Select Pin*/
	uint8_t				CSPin;			/*GPIO Pin for Device Chip Select Pin*/
	uint16_t			PageSize;		/*Page Size of Memory*/
	uint64_t			MemSize;		/*Size of Memory in byte*/
}EEPROM_Config_t;

extern EEPROM_Config_t EEPROM_Dev_List[];

extern M95_status_t M95_Init(SPI_HandleTypeDef *hspi);
extern M95_status_t M95_Read(SPI_HandleTypeDef *hspi, EEPROM_Config_t *pEEPROM, uint32_t Start_Address, uint16_t Read_Size, uint8_t *pData);
extern M95_status_t M95_Write(SPI_HandleTypeDef *hspi, EEPROM_Config_t *pEEPROM, uint32_t Start_Address, uint16_t Write_Size, uint8_t *pData);
extern M95_status_t M95_Clear(SPI_HandleTypeDef *hspi, uint8_t *pData);
extern M95_status_t M95_Wait_Until_WIP(SPI_HandleTypeDef *hspi);
#endif /* M95_EEPROM_H_ */
