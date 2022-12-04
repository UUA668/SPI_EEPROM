/*
 * M95_EEPROM.h
 *
 *  Created on: 24.11.2022
 *      Author: markfe1
 */

#ifndef M95_EEPROM_H_
#define M95_EEPROM_H_

#include "stm32g0xx_hal.h"

#define EEPROM_SIZE 512
#define PAGE_SIZE 16

/*M95 Status definicio*/
typedef enum {
	M95_OK = 0,
	M95_NOK,
} M95_status_t;


M95_status_t M95_Init(SPI_HandleTypeDef *hspi);
M95_status_t M95_Read(SPI_HandleTypeDef *hspi,uint8_t *pData);
M95_status_t M95_Clear(SPI_HandleTypeDef *hspi, uint8_t *pData);
M95_status_t M95_Wait_Until_WIP(SPI_HandleTypeDef *hspi);
#endif /* M95_EEPROM_H_ */
