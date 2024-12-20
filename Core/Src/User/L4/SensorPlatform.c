/*
 * remoteSensingPlatform.c
 *
 *  Created on: Oct. 21, 2022
 *      Author: Andre Hendricks / Dr. JF Bousquet
 */
#include <stdio.h>

#include "User/L2/Comm_Datalink.h"

#include "User/L3/AcousticSensor.h"
#include "User/L3/CorrosionSensor.h"
#include "User/L3/FlowRateSensor.h"
#include "User/L3/PressureSensor.h"

// New sensors
#include "User/L3/Acoustic2Sensor.h"
#include "User/L3/Pressure2Sensor.h"
#include "User/L3/HallEffectSensor.h"
#include "User/L3/TemperatureSensor.h"

#include "User/L4/SensorPlatform.h"

#include "User/util.h"

//Required FreeRTOS header files
#include "FreeRTOS.h"
#include "Timers.h"
#include "semphr.h"

static void ResetMessageStruct(struct CommMessage* currentRxMessage){

	static const struct CommMessage EmptyMessage = {0};
	*currentRxMessage = EmptyMessage;
}

/******************************************************************************
This task is created from the main.
It is responsible for managing the messages from the datalink.
It is also responsible for starting the timers for each sensor
******************************************************************************/
void SensorPlatformTask(void *params)
{
	const TickType_t TimerDefaultPeriod = 10000;
	char str[60];
	TimerHandle_t TimerID_AcousticSensor,TimerID_PressureSensor, TimerID_FlowRateSensor, TimerID_CorrosionSensor;
	TimerHandle_t TimerID_HallEffectSensor, TimerID_TemperatureSensor;

	TimerID_AcousticSensor = xTimerCreate("Acoustic Sensor Task", TimerDefaultPeriod, pdTRUE, (void*)2, RunAcousticSensor);
	TimerID_PressureSensor = xTimerCreate("Pressure Sensor Task", TimerDefaultPeriod, pdTRUE, (void*)3, RunPressureSensor);
	// TimerID_FlowRateSensor = xTimerCreate("Flow Rate Sensor Task", TimerDefaultPeriod, pdTRUE, (void*)2, RunFlowRateSensor);
	//TimerID_CorrosionSensor = xTimerCreate("Corrosion Sensor Task", TimerDefaultPeriod, pdTRUE, (void*)3, RunCorrosionSensor);

	TimerID_HallEffectSensor = xTimerCreate("Hall Effect Sensor Task", TimerDefaultPeriod, pdTRUE, (void*)5, RunHallEffectSensor);
	TimerID_TemperatureSensor = xTimerCreate("Temperature Sensor Task", TimerDefaultPeriod, pdTRUE, (void*)4, RunTemperatureSensor);

	// Verify timer creation
	sprintf(str, "Timer creation status:\r\n");
	print_str(str);
	sprintf(str, "Acoustic: %d\r\n", TimerID_AcousticSensor != NULL);
	print_str(str);
	sprintf(str, "Pressure: %d\r\n", TimerID_PressureSensor != NULL);
	print_str(str);
	sprintf(str, "HallEffect: %d\r\n", TimerID_HallEffectSensor != NULL);
	print_str(str);
	sprintf(str, "Temperature: %d\r\n", TimerID_TemperatureSensor != NULL);
	print_str(str);

	request_sensor_read();  // requests a usart read (through the callback)

	struct CommMessage currentRxMessage = {0};

	do {

			parse_sensor_message(&currentRxMessage);

			if(currentRxMessage.IsMessageReady == true && currentRxMessage.IsCheckSumValid == true){

				switch(currentRxMessage.SensorID){
					case Controller:
						switch(currentRxMessage.messageId){
							case 0:
								xTimerStop(TimerID_AcousticSensor,  portMAX_DELAY);
								xTimerStop(TimerID_PressureSensor,  portMAX_DELAY);
								// xTimerStop(TimerID_FlowRateSensor,  portMAX_DELAY);
								//xTimerStop(TimerID_CorrosionSensor, portMAX_DELAY);
								xTimerStop(TimerID_HallEffectSensor, portMAX_DELAY);
								xTimerStop(TimerID_TemperatureSensor, portMAX_DELAY);
								send_ack_message(RemoteSensingPlatformReset);
								break;
							case 1: //Do Nothing
								break;
							case 3: //Do Nothing
								break;
						}
						break;
					case Acoustic:
						switch(currentRxMessage.messageId){
							case 0:
								xTimerChangePeriod(TimerID_AcousticSensor, currentRxMessage.params, portMAX_DELAY);
								sprintf(str, "Setting Acoustic timer period to %d\r\n", currentRxMessage.params);
								print_str(str);
								xTimerStart(TimerID_AcousticSensor, portMAX_DELAY);
								if(xTimerIsTimerActive(TimerID_AcousticSensor) == pdTRUE) {
									sprintf(str, "Acoustic timer successfully started\r\n");
									print_str(str);
								}
								send_ack_message(AcousticSensorEnable);
								break;
							case 1: //Do Nothing
								break;
							case 3: //Do Nothing
								break;
						}
						break;
					case Pressure:
						switch(currentRxMessage.messageId){
							case 0:
								xTimerChangePeriod(TimerID_PressureSensor, currentRxMessage.params, portMAX_DELAY);
								xTimerStart(TimerID_PressureSensor, portMAX_DELAY);
								send_ack_message(PressureSensorEnable);
								break;
							case 1: //Do Nothing
								break;
							case 3: //Do Nothing
								break;
						}
						break;
					// case FlowRate:
					// 	switch(currentRxMessage.messageId){
					// 		case 0:
					// 			xTimerChangePeriod(TimerID_FlowRateSensor, currentRxMessage.params, portMAX_DELAY);
					// 			xTimerStart(TimerID_FlowRateSensor, portMAX_DELAY);
					// 			send_ack_message(FlowRateSensorEnable);
					// 			break;
					// 		case 1: //Do Nothing
					// 			break;
					// 		case 3: //Do Nothing
					// 			break;
					// 	}
					// 	break;
					/*case Corrosion:
						switch(currentRxMessage.messageId){
							case 0:
								xTimerChangePeriod(TimerID_CorrosionSensor, currentRxMessage.params, portMAX_DELAY);
								xTimerStart(TimerID_CorrosionSensor, portMAX_DELAY);
								send_ack_message(CorrosionSensorEnable);
								break;
							case 1: //Do Nothing
								break;
							case 3: //Do Nothing
								break;
						}
						break;*/
					// case Acoustic2:
					// 	switch(currentRxMessage.messageId){
					// 		case 0:
					// 			xTimerChangePeriod(TimerID_Acoustic2Sensor, currentRxMessage.params, portMAX_DELAY);
					// 			xTimerStart(TimerID_Acoustic2Sensor, portMAX_DELAY);
					// 			send_ack_message(Acoustic2SensorEnable);
					// 			break;
					// 		case 1: //Do Nothing
					// 			break;
					// 		case 3: //Do Nothing
					// 			break;
					// 	}
					// 	break;
					// case Pressure2:
					// 	switch(currentRxMessage.messageId){
					// 		case 0:
					// 			xTimerChangePeriod(TimerID_Pressure2Sensor, currentRxMessage.params, portMAX_DELAY);
					// 			xTimerStart(TimerID_Pressure2Sensor, portMAX_DELAY);
					// 			send_ack_message(Pressure2SensorEnable);
					// 			break;
					// 		case 1: //Do Nothing
					// 			break;
					// 		case 3: //Do Nothing
					// 			break;
					// 	}
					// 	break;
					case HallEffect:
						switch(currentRxMessage.messageId){
							case 0:
								xTimerChangePeriod(TimerID_HallEffectSensor, currentRxMessage.params, portMAX_DELAY);
								xTimerStart(TimerID_HallEffectSensor, portMAX_DELAY);
								send_ack_message(HallEffectSensorEnable);
								break;
							case 1: //Do Nothing
								break;
							case 3: //Do Nothing
								break;
						}
						break;
					case Temperature:
						switch(currentRxMessage.messageId){
							case 0:
								xTimerChangePeriod(TimerID_TemperatureSensor, currentRxMessage.params, portMAX_DELAY);
								xTimerStart(TimerID_TemperatureSensor, portMAX_DELAY);
								send_ack_message(TemperatureSensorEnable);
								break;
							case 1: //Do Nothing
								break;
							case 3: //Do Nothing
								break;
						}
						break;
					default://Should not get here
						break;
				}
				ResetMessageStruct(&currentRxMessage);
			}
		} while(1);
}
