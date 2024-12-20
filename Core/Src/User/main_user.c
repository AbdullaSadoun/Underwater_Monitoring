/*
 * main_user.c
 *
 *  Created on: Aug 8, 2022
 *      Author: Andre Hendricks
 */

#include <stdio.h>

//STM32 generated header files
#include "main.h"

//User generated header files
#include "User/main_user.h"
#include "User/util.h"
#include "User/L1/USART_Driver.h"
#include "User/L2/Comm_Datalink.h"
#include "User/L4/SensorPlatform.h"
#include "User/L4/SensorController.h"

//Required FreeRTOS header files
#include "FreeRTOS.h"
#include "task.h"

#define SENSORPLATFORM_MODE 0
#define SENSORCONTROLLER_MODE 1

// for the board being powered (sensors)
//#define CODE_MODE SENSORPLATFORM_MODE

// for the board powering (controller)
#define CODE_MODE SENSORCONTROLLER_MODE

void main_user(){
	util_init();

	initialize_sensor_datalink();

#if CODE_MODE == SENSORCONTROLLER_MODE
	initialize_hostPC_datalink();
#endif

#if CODE_MODE == SENSORCONTROLLER_MODE
	xTaskCreate(HostPC_RX_Task,"HostPC_RX_Task", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 2, NULL);

	xTaskCreate(SensorPlatform_RX_Task,"SensorPlatform_RX_Task", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 2, NULL);

	xTaskCreate(SensorControllerTask,"Sensor_Controller_Task", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 2, NULL);
#elif CODE_MODE == SENSORPLATFORM_MODE
	xTaskCreate(SensorPlatformTask,"Sensor_Platform_Task", configMINIMAL_STACK_SIZE + 400, NULL, tskIDLE_PRIORITY + 2, NULL);
#endif
	vTaskStartScheduler();

	while(1);
	

}
