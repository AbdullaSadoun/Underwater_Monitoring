/*
 * USART_Driver.h
 *
 *  Created on: Oct 22, 2022
 *      Author: kadh1
 */

#ifndef INC_USER_L1_USART_DRIVER_H_
#define INC_USER_L1_USART_DRIVER_H_

#include "FreeRTOS.h"
#include "queue.h"

extern QueueHandle_t Queue_extern_UART;
extern QueueHandle_t Queue_hostPC_UART;

void configure_usart_extern(void);
void configure_usart_hostPC(void);

void request_sensor_read(void);
void request_hostPC_read(void);

void printStr_extern(char * str);

#endif /* INC_USER_L1_USART_DRIVER_H_ */
