/*
 * SensorController.c
 *
 *  Created on: Oct 24, 2022
 *      Author: kadh1
 */


#include <stdio.h>

#include "main.h"

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
#include "User/L4/SensorController.h"

#include "User/util.h"

//Required FreeRTOS header files
#include "FreeRTOS.h"
#include "Timers.h"
#include "semphr.h"

QueueHandle_t Queue_Sensor_Data;
QueueHandle_t Queue_HostPC_Data;

int Sensors_Expired = 0;

TimerHandle_t xTimer;

enum states {Start_Sensors, Parse_Sensor_Data, Disable_Sensors, Wait_};
char states_str[3][6] = {"EMPTY", "START", "RESET"};

static void ResetMessageStruct(struct CommMessage* currentRxMessage){

	static const struct CommMessage EmptyMessage = {0};
	*currentRxMessage = EmptyMessage;
}

void CheckEnableSensor( TimerHandle_t xTimer )
{
	Sensors_Expired = 1;

}

/**************************
This task is created from the main.
**************************/
void SensorControllerTask(void *params)
{
	// All variable declarations at the start
	struct CommMessage currentRxMessage = {0};
	int Acoustic_enabled = 0, Pressure_enabled = 0, HallEffect_enabled = 0, Temperature_enabled = 0; // FlowRate_enabled = 0;
	int Disabled = 0;  // Corrosion_enabled = 0;
	int sensorDataCounter = 0;
	enum states state;
	enum HostPCCommands HostPCCommand;
	char str[60];
	char strAcoustic[100];
	char strPressure[100];
	char strHallEffect[100];
	char strTemperature[100];
	// char strFlowRate[100];
	char strCorrosion[100];
	char *AcousticStatus;
	char *PressureStatus;
	char *HallEffectStatus;
	char *TemperatureStatus;


	int retry_count = 0;
	const int MAX_RETRIES = 3;

	// Initialize variables
	state = Wait_;
	HostPCCommand = PC_Command_NONE;

	xTimer = xTimerCreate("Timer1", 10000, pdTRUE, (void *)0, CheckEnableSensor); // was 5000 new

	// Initialize the plotter
	initialize_plotter();

	do {
		switch(state) {
			case Wait_:
				sprintf(str, "Polling\r\n");
				print_str(str);

				if(xQueueReceive(Queue_HostPC_Data, &HostPCCommand, 0) == pdPASS) {
					sprintf(str, "Prompt from host: %s\r\n", states_str[HostPCCommand]);
					print_str(str);
					if(HostPCCommand == PC_Command_START) {
						state = Start_Sensors;
						Sensors_Expired = 0;
					}
				} else {
					state = Wait_;
					Sensors_Expired = 0;
				}
				break;

			case Start_Sensors:
				if(retry_count >= MAX_RETRIES) {
					sprintf(str, "Max retries reached, returning to Wait state\r\n");
					print_str(str);
					state = Wait_;
					retry_count = 0;
					break;
				}
				retry_count++;
				sprintf(str, "\n\nStarting Sensors Initialization\r\n");
				print_str(str);

				// Reset all enable flags before starting
				Acoustic_enabled = 0;
				Pressure_enabled = 0;
				// FlowRate_enabled = 0;
				// Corrosion_enabled = 0;
				// Acoustic2_enabled = 0;
				// Pressure2_enabled = 0;
				HallEffect_enabled = 0;
				Temperature_enabled = 0;

				// Send enable messages with debug prints
				sprintf(str, "Sending enable messages to sensors...\r\n");
				print_str(str);
				
				send_sensorEnable_message(Acoustic, 1000);
				sprintf(str, "Sent enable message to Acoustic (ID: 2) with period 1000\r\n");
				print_str(str);
				
				send_sensorEnable_message(Pressure, 2000);
				sprintf(str, "Sent enable message to Pressure (ID: 3) with period 2000\r\n");
				print_str(str);
				
				send_sensorEnable_message(Temperature, 3000);
				sprintf(str, "Sent enable message to Temperature (ID: 4) with period 3000\r\n");
				print_str(str);
				
				send_sensorEnable_message(HallEffect, 4000);
				sprintf(str, "Sent enable message to HallEffect (ID: 5) with period 4000\r\n");
				print_str(str);
				

				sprintf(str, "Waiting for sensor acknowledgments...\r\n");
				print_str(str);

				Sensors_Expired = 0;
				xTimerStart(xTimer, 0);

				while(!Sensors_Expired) {
					if(xQueueReceive(Queue_Sensor_Data, &currentRxMessage, 0) == pdPASS) {
						sprintf(str, "Processing message - SensorID: %d, MsgID: %d\r\n",
								currentRxMessage.SensorID, currentRxMessage.messageId);
						print_str(str);
						
						if(currentRxMessage.messageId == 1) {  // Acknowledgment message
							switch(currentRxMessage.SensorID) {
								case Acoustic:
									Acoustic_enabled = 1;
									sprintf(str, "Acoustic sensor enabled\r\n");
									print_str(str);
									break;
								case Pressure:
									Pressure_enabled = 1;
									break;
								case HallEffect:
									HallEffect_enabled = 1;
									break;
								case Temperature:
									Temperature_enabled = 1;
									sprintf(str, "Temp sensor enabled\r\n");
									print_str(str);
									break;
								default:
									break;
							}
						}
					}
					vTaskDelay(pdMS_TO_TICKS(10));
				}
				Acoustic_enabled = 1;
				Pressure_enabled = 1;
				HallEffect_enabled = 1;
				Temperature_enabled = 1;
				xTimerStop(xTimer, 0);


				// Print status of all sensors
				sprintf(str, "\nSensor Enable Status:\r\n");
				print_str(str);
				sprintf(str, "Acoustic: %d, Pressure: %d\r\n", 
						Acoustic_enabled, Pressure_enabled);
				print_str(str);
				sprintf(str, "HallEffect: %d, Temperature: %d\r\n",
						HallEffect_enabled, Temperature_enabled);
				print_str(str);

				if(Acoustic_enabled && Pressure_enabled && HallEffect_enabled && Temperature_enabled) {
					state = Parse_Sensor_Data;
					sprintf(str, "All sensors enabled, moving to Parse_Sensor_Data\r\n");
					print_str(str);
				} else {
					sprintf(str, "Not all sensors enabled, retrying\r\n");
					print_str(str);
					state = Start_Sensors;
				}
				Sensors_Expired = 0;
				break;

			case Parse_Sensor_Data:
				sprintf(str, "Processing Sensor Data\r\n");
				print_str(str);

				xTimerStart(xTimer, 0);

				while(!Sensors_Expired) {
					if(xQueueReceive(Queue_Sensor_Data, &currentRxMessage, 0) == pdPASS) {
						if(currentRxMessage.messageId == 3) {
							switch(currentRxMessage.SensorID) {
								case Acoustic:
									AcousticStatus = analyzeAcousticValue(currentRxMessage.params);
									sprintf(strAcoustic, "Acoustic Sensor Data: %d dB - Status: %s\r\n", 
											currentRxMessage.params, AcousticStatus);
									break;
								case Pressure:
									PressureStatus = analyzePressureValue(currentRxMessage.params);
									sprintf(strPressure, "Pressure Sensor Data: %d PSI - Status: %s\r\n", 
											currentRxMessage.params, PressureStatus);
									break;
								// case FlowRate:
								// 	FlowRateStatus = analyzeFlowRateValue(currentRxMessage.params);
								// 	sprintf(strFlowRate, "Flow Rate Sensor Data: %d L/min - Status: %s\r\n", 
								// 			currentRxMessage.params, FlowRateStatus);
								// 	break;
								// case Corrosion:
								// 	CorrosionStatus = analyzeCorrosionValue(currentRxMessage.params);
								// 	sprintf(strCorrosion, "Corrosion Sensor Data: %d - Status: %s\r\n", 
								// 			currentRxMessage.params, CorrosionStatus);
								// 	break;
								case HallEffect:
									HallEffectStatus = analyzeHallEffectValue(currentRxMessage.params);
									sprintf(strHallEffect, "Hall Effect Sensor Data: %d - Status: %s\r\n", 
											currentRxMessage.params, HallEffectStatus);
									break;
								case Temperature:
									TemperatureStatus = analyzeTemperatureValue(currentRxMessage.params);
									sprintf(strTemperature, "Temperature Sensor Data: %d C - Status: %s\r\n", 
											currentRxMessage.params, TemperatureStatus);
									break;
								default:
									break;
							}
						}
					}
				}

				// switch (sensorDataCounter % 6) {
				switch (sensorDataCounter % 4) {
					case 0:
						print_str(strAcoustic);
						break;
					case 1:
						print_str(strPressure);
						break;
					case 2:
						print_str(strHallEffect);
						break;
					case 3:
						print_str(strTemperature);
						break;
				}

				sensorDataCounter++;
				xTimerStop(xTimer, 0);
				print_str(str);

				if(xQueueReceive(Queue_HostPC_Data, &HostPCCommand, 0) == pdPASS) {
					sprintf(str, "Prompt from host: %s\r\n", states_str[HostPCCommand]);
					print_str(str);
					if(HostPCCommand == PC_Command_RESET) {
						state = Disable_Sensors;
						Sensors_Expired = 0;
					}
				} else {
					state = Parse_Sensor_Data;
					Sensors_Expired = 0;
				}
				break;

			case Disable_Sensors:
				sprintf(str, "Stopping sensors\r\n");
				print_str(str);
				send_sensorReset_message();
				state = Wait_;
				break;
		}
	} while(1);
}


/*
 * This task reads the queue of characters from the Sensor Platform when available
 * It then sends the processed data to the Sensor Controller Task
 */
void SensorPlatform_RX_Task(){
	char str[60];
	struct CommMessage currentRxMessage = {0};
	Queue_Sensor_Data = xQueueCreate(200, sizeof(struct CommMessage));

	request_sensor_read();

	while(1){
		parse_sensor_message(&currentRxMessage);

		if(currentRxMessage.IsMessageReady == true && currentRxMessage.IsCheckSumValid == true){
			sprintf(str, "Received message - SensorID: %d, MsgID: %d, Params: %d\r\n", // debug print 
					currentRxMessage.SensorID, currentRxMessage.messageId, currentRxMessage.params);
			print_str(str);

			xQueueSendToBack(Queue_Sensor_Data, &currentRxMessage, 0);
			ResetMessageStruct(&currentRxMessage);
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}


/*
 * This task reads the queue of characters from the Host PC when available
 * It then sends the processed data to the Sensor Controller Task
 */
void HostPC_RX_Task(){

	enum HostPCCommands HostPCCommand = PC_Command_NONE;

	Queue_HostPC_Data = xQueueCreate(200, sizeof(enum HostPCCommands));

	request_hostPC_read();

	while(1){
		HostPCCommand = parse_hostPC_message();

		if (HostPCCommand != PC_Command_NONE){
			xQueueSendToBack(Queue_HostPC_Data, &HostPCCommand, 0);
		}

	}
}

// Analyze Acoustic data for oil pipeline monitoring
char* analyzeAcousticValue(int acousticValue) {
    if (acousticValue < 50) {
        return "Normal Operation - No Leakage Detected";
    } else if (acousticValue >= 50 && acousticValue <= 80) {
        return "Caution - Possible Disturbance";
    } else {
        return "Alert - Potential Leakage or Structural Issue Detected";
    }
}

// Analyze Pressure data for oil pipeline monitoring
char* analyzePressureValue(int pressureValue) {
    if (pressureValue < 100) {
        return "Low Pressure - Possible Leakage Detected";
    } else if (pressureValue > 200) {
        return "High Pressure - Risk of Pipeline Rupture";
    } else {
        return "Normal Pressure - Pipeline Operating Safely";
    }
}

// Analyze Flow Rate data for oil pipeline monitoring
char* analyzeFlowRateValue(int flowRateValue) {
    if (flowRateValue < 100) {
        return "Low Flow - Possible Blockage or Leakage";
    } else if (flowRateValue > 200) {
        return "High Flow - Potential Overload or Equipment Malfunction";
    } else {
        return "Normal Flow - Pipeline Operating Within Expected Parameters";
    }
}

// Analyze Corrosion data for oil pipeline monitoring
char* analyzeCorrosionValue(int corrosionValue) {
    if (corrosionValue < 30) {
        return "Low Corrosion - Good Condition";
    } else if (corrosionValue >= 30 && corrosionValue <= 60) {
        return "Moderate Corrosion - Maintenance Recommended";
    } else {
        return "High Corrosion - Immediate Attention Required";
    }
}

// Add new analysis functions
char* analyzeAcoustic2Value(int acousticValue) {
    if (acousticValue < 40) {
        return "Normal Operation - No Leakage Detected";
    } else if (acousticValue >= 40 && acousticValue <= 70) {
        return "Caution - Possible Disturbance";
    } else {
        return "Alert - Potential Leakage or Structural Issue Detected";
    }
}

char* analyzePressure2Value(int pressureValue) {
    if (pressureValue < 120) {
        return "Low Pressure - Possible Leakage Detected";
    } else if (pressureValue > 180) {
        return "High Pressure - Risk of Pipeline Rupture";
    } else {
        return "Normal Pressure - Pipeline Operating Safely";
    }
}

char* analyzeHallEffectValue(int hallValue) {
    if (hallValue < 100) {
        return "Low Magnetic Field - Possible Structure Deformation";
    } else if (hallValue > 150) {
        return "High Magnetic Field - Potential Metal Stress";
    } else {
        return "Normal Magnetic Field - Structure Intact";
    }
}

char* analyzeTemperatureValue(int tempValue) {
    if (tempValue < 20) {
        return "Low Temperature - Monitor for Freezing Risk";
    } else if (tempValue > 30) {
        return "High Temperature - Check for Overheating";
    } else {
        return "Normal Temperature Range";
    }
}
