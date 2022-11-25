/*
 * M95_EEPROM.h
 *
 *  Created on: 24.11.2022
 *      Author: markfe1
 */

#ifndef M95_EEPROM_H_
#define M95_EEPROM_H_

#include "stm32g0xx_hal.h"

/*M95 Status definicio*/
typedef enum {
	M95_OK = 0,
	M95_NOK,
} M95_status_t;


M95_status_t M95_Init(SPI_HandleTypeDef *hspi, uint8_t *pBuffer_RDSR);
M95_status_t M95_Read(SPI_HandleTypeDef *hspi,uint8_t *pData);

#endif /* M95_EEPROM_H_ */
