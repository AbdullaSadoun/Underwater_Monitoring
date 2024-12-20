/*
 * Comm_Datalink.c
 *
 *  Created on: Oct. 21, 2022
 *      Author: Andre Hendricks / Dr. JF Bousquet
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "User/L1/USART_Driver.h"
#include "User/L2/Comm_Datalink.h"
#include "User/util.h"

enum ParseMessageState_t {Waiting_S, SensorID_S, MessageID_S, ParamsID_S, Star_S, CS_S};

static void sendStringSensor(char* tx_string);

/******************************************************************************
******************************************************************************/
void initialize_sensor_datalink(void)
{
	configure_usart_extern();
}

void initialize_hostPC_datalink(void){
	configure_usart_hostPC();
}

/******************************************************************************
this function calculates the checksum and sends a given input string to the uart.
******************************************************************************/
static void sendStringSensor(char* tx_string)
{
	uint8_t checksum;
	uint16_t str_length;

	// Check the length of the command
	str_length = strlen((char *)tx_string)-1;

	// Compute the checksum
	checksum = tx_string[0];
	for (int idx = 1; idx < str_length-2; idx++) {
		checksum ^= tx_string[idx];
	}

	sprintf(&tx_string[str_length-2], "%02x\n", checksum);
	printStr_extern(tx_string);
}


/******************************************************************************
This function parses the queue to identify received messages.
******************************************************************************/
void parse_sensor_message(struct CommMessage* currentRxMessage) // updated to include new sensors
{
	static enum ParseMessageState_t currentState = Waiting_S;
	uint8_t CurrentChar;
	static uint16_t sensorIdIdx = 0, MessageIdIdx = 0, ParamIdx = 0, checksumIdx = 0;
	static char sensorId[6],CSStr[3];
	static uint8_t checksum_val;
	static const struct CommMessage EmptyMessage = {0};

	while(xQueueReceive(Queue_extern_UART, &CurrentChar, portMAX_DELAY) == pdPASS && currentRxMessage->IsMessageReady == false)  // as long as there are characters in the queue.
	{
		if (CurrentChar == '$'){ //Reset State Machine
			checksum_val = CurrentChar;
			sensorIdIdx = 0;
			MessageIdIdx = 0;
			ParamIdx = 0;
			checksumIdx = 0;
			currentState = SensorID_S;
			*currentRxMessage = EmptyMessage;
			continue;
		}

		// TO DO: we must calculate the received checksum!
		switch (currentState)
		{
			case Waiting_S: // Do nothing
				break;
			case SensorID_S: //Get Sensor ID Code
				checksum_val ^= CurrentChar;
				if(CurrentChar == ','){
					currentState = MessageID_S;
					break;
				}
				else if (sensorIdIdx < 5){
					sensorId[sensorIdIdx++] = CurrentChar;
				}
				if(sensorIdIdx == 5){
				    // Add NULL Terminator
				    sensorId[sensorIdIdx] = '\0';

				    if(strcmp(sensorId, "CNTRL") == 0) // Sensor ID: Controller
				        currentRxMessage->SensorID = Controller;
				    else if(strcmp(sensorId, "ACST1") == 0) // Sensor ID: Acoustic
				        currentRxMessage->SensorID = Acoustic;
				    // else if(strcmp(sensorId, "ACST2") == 0) // Sensor ID: Acoustic2
				    //     currentRxMessage->SensorID = Acoustic2;
				    else if(strcmp(sensorId, "PRS01") == 0) // Sensor ID: Pressure
				        currentRxMessage->SensorID = Pressure;
				    // else if(strcmp(sensorId, "PRS02") == 0) // Sensor ID: Pressure2
				    //     currentRxMessage->SensorID = Pressure2;
				    // else if(strcmp(sensorId, "FLRAT") == 0) // Sensor ID: Flow Rate
				    //     currentRxMessage->SensorID = FlowRate;
				    // else if(strcmp(sensorId, "CORRN") == 0) // Sensor ID: Corrosion
				    //     currentRxMessage->SensorID = Corrosion;
				    else if(strcmp(sensorId, "HZ21W") == 0) // Sensor ID: Hall Effect
				        currentRxMessage->SensorID = HallEffect;
				    else if(strcmp(sensorId, "DS18B") == 0) // Sensor ID: Temperature
				        currentRxMessage->SensorID = Temperature;
				    else { // Sensor ID: None
				        currentRxMessage->SensorID = None;
				        currentState = Waiting_S;
				    }
				}
				break;

			case MessageID_S: //Get Message Type
				checksum_val ^= CurrentChar;
				if(CurrentChar == ','){
					currentState = ParamsID_S;
				}
				else{
					if(MessageIdIdx < 2){
						currentRxMessage->messageId = currentRxMessage->messageId * 10;
						currentRxMessage->messageId += CurrentChar -  '0';
					}
					MessageIdIdx++;
				}
				break;

			case ParamsID_S: //Get Message Parameter (Period/Data)
				checksum_val ^= CurrentChar;

				if(CurrentChar == ','){
					currentState = Star_S;
				}
				else if(ParamIdx < 8){
					currentRxMessage->params = currentRxMessage->params * 10;
					currentRxMessage->params += CurrentChar -  '0';
				}
				break;

			case Star_S:
				checksum_val ^= CurrentChar;
				if(CurrentChar == ','){
					currentState = CS_S;
				}
				break;

			case CS_S:
				if(checksumIdx < 2){
					CSStr[checksumIdx++] = CurrentChar;
				}
				if(checksumIdx == 2){
					currentState = Waiting_S;
					CSStr[checksumIdx] = '\0';
					currentRxMessage->checksum = strtol(CSStr, NULL, 16);
					if(currentRxMessage->checksum == checksum_val){
						currentRxMessage->IsMessageReady = true;
						currentRxMessage->IsCheckSumValid = true;
					}else{
						currentRxMessage->IsCheckSumValid = false;
					}
				}
					break;
			}
		}

	if(currentRxMessage->IsMessageReady == true) {
		char str[100];
		// sprintf(str, "Parsed message: ID=%d, MsgID=%d, Params=%d, Checksum=%d\r\n", // debug print
		// 		currentRxMessage->SensorID, currentRxMessage->messageId,
		// 		currentRxMessage->params, currentRxMessage->checksum);
		// print_str(str);

	}
}



enum HostPCCommands parse_hostPC_message(){

	uint8_t CurrentChar;
	static char HostPCMessage[10];
	static uint16_t HostPCMessage_IDX = 0;


	while (xQueueReceive(Queue_hostPC_UART, &CurrentChar, portMAX_DELAY) == pdPASS){
		if(CurrentChar == '\n' || CurrentChar == '\r'|| HostPCMessage_IDX >=6){
			HostPCMessage[HostPCMessage_IDX++] = '\0';
			HostPCMessage_IDX = 0;
			if(strcmp(HostPCMessage, "START") == 0)
				return PC_Command_START;
			else if(strcmp(HostPCMessage, "RESET") == 0)
				return PC_Command_RESET;
		}else{
			HostPCMessage[HostPCMessage_IDX++] = CurrentChar;
		}

	}
	return PC_Command_NONE;
}


void send_sensorData_message(enum SensorId_t sensorType, uint16_t data)
{
    char tx_sensor_buffer[50];
    char str[60];

    switch(sensorType){
    case Acoustic:
        sprintf(tx_sensor_buffer, "$ACST1,03,%08u,*,00\n", data);
        sprintf(str, "Sending Acoustic data: %s", tx_sensor_buffer);
        print_str(str);
        break;
    case Pressure:
        sprintf(tx_sensor_buffer, "$PRS01,03,%08u,*,00\n", data);
		
        sprintf(str, "Sending Pressure data: %s", tx_sensor_buffer);
        print_str(str);
        break;
    // case FlowRate:
    //     sprintf(tx_sensor_buffer, "$FLRAT,03,%08u,*,00\n", data);
    //     break;
    // case Corrosion:
    //     sprintf(tx_sensor_buffer, "$CORRN,03,%08u,*,00\n", data);
    //     break;
    // // New sensors
    // case Acoustic2: // New sensor
    //     sprintf(tx_sensor_buffer, "$ACSTC,03,%08u,*,00\n", data);
    //     break;
    // case Pressure2: // New sensor
    //     sprintf(tx_sensor_buffer, "$PRESS,03,%08u,*,00\n", data);
    //     break;	
    case HallEffect: // New sensor
        sprintf(tx_sensor_buffer, "$HZ21W,03,%08u,*,00\n", data);
        break;
    case Temperature: // New sensor
        sprintf(tx_sensor_buffer, "$DS18B,03,%08u,*,00\n", data);
        break;
    default:
        // Handle unknown sensor type or add a default case if needed
        break;
    }
    sendStringSensor(tx_sensor_buffer);
}

void send_sensorEnable_message(enum SensorId_t sensorType, uint16_t TimePeriod){
    char tx_sensor_buffer[50];
    char str[100];

    switch(sensorType){
    case Acoustic:
        sprintf(tx_sensor_buffer, "$ACST1,00,%08u,*,00\n", TimePeriod);
		//sprintf(tx_sensor_buffer, "$ACSTC,03,%08u,*,00\n", TimePeriod);
        break;
    // case Acoustic2:
    //     sprintf(tx_sensor_buffer, "$ACST2,00,%08u,*,00\n", TimePeriod);
    //     break;
    case Pressure:
        sprintf(tx_sensor_buffer, "$PRS01,00,%08u,*,00\n", TimePeriod);
        break;
    // case Pressure2:
    //     sprintf(tx_sensor_buffer, "$PRS02,00,%08u,*,00\n", TimePeriod);
    //     break;
    // case FlowRate:
    //     sprintf(tx_sensor_buffer, "$FLRAT,00,%08u,*,00\n", TimePeriod);
    //     break;
    // case Corrosion:
    //     sprintf(tx_sensor_buffer, "$CORRN,00,%08u,*,00\n", TimePeriod);
    //     break;
    case HallEffect:
        sprintf(tx_sensor_buffer, "$HZ21W,00,%08u,*,00\n", TimePeriod);
        break;
    case Temperature:
        sprintf(tx_sensor_buffer, "$DS18B,00,%08u,*,00\n", TimePeriod);
        break;
    default:
        // Optional: Handle unknown sensor type or add a default case if needed
        break;
    }
    sendStringSensor(tx_sensor_buffer);
    vTaskDelay(pdMS_TO_TICKS(50));  // Add delay between messages
}


void send_sensorReset_message(void){
	char tx_sensor_buffer[50];

	sprintf(tx_sensor_buffer, "$CNTRL,00,,*,00\n");

	sendStringSensor(tx_sensor_buffer);
}

void send_ack_message(enum AckTypes AckType){ // updated to include new sensors
    char tx_sensor_buffer[50];

    switch(AckType){
    case RemoteSensingPlatformReset:
        sprintf(tx_sensor_buffer, "$CNTRL,01,,*,00\n");
        break;
    case AcousticSensorEnable:
        sprintf(tx_sensor_buffer, "$ACST1,01,,*,00\n");
        break;
    // case Acoustic2SensorEnable:
    //     sprintf(tx_sensor_buffer, "$ACST2,01,,*,00\n");
    //     break;
    case PressureSensorEnable:
        sprintf(tx_sensor_buffer, "$PRS01,01,,*,00\n");
        break;
    // case Pressure2SensorEnable:
    //     sprintf(tx_sensor_buffer, "$PRS02,01,,*,00\n");
    //     break;
    // case FlowRateSensorEnable:
    //     sprintf(tx_sensor_buffer, "$FLRAT,01,,*,00\n");
    //     break;
    // case CorrosionSensorEnable:
    //     sprintf(tx_sensor_buffer, "$CORRN,01,,*,00\n");
    //     break;
    // New sensors
    case HallEffectSensorEnable: // New sensor
        sprintf(tx_sensor_buffer, "$HZ21W,01,,*,00\n");
        break;
    case TemperatureSensorEnable: // New sensor
        sprintf(tx_sensor_buffer, "$DS18B,01,,*,00\n");
        break;
    default:
        // Optional: Handle unknown sensor type or add a default case if needed
        break;
    }

    sendStringSensor(tx_sensor_buffer);
}

