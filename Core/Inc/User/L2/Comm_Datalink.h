/*
 * Comm_Datalink.h
 *
 *  Created on: Oct 22, 2022
 *      Author: kadh1
 */

#ifndef INC_USER_L2_COMM_DATALINK_H_
#define INC_USER_L2_COMM_DATALINK_H_


#include <stdbool.h>

#include "User/L1/USART_Driver.h"

#include "FreeRTOS.h"
#include "semphr.h"

enum SensorId_t {
    None,
    Controller,
    Acoustic,    // ID: 2
    Pressure,    // ID: 3
    Temperature, // ID: 4
    HallEffect,  // ID: 5
};

enum AckTypes {
    RemoteSensingPlatformReset,
    AcousticSensorEnable,      // Existing
    PressureSensorEnable,      // Existing
    // FlowRateSensorEnable,      // Existing
    // CorrosionSensorEnable,     // Existing
    HallEffectSensorEnable,    // SEN-HZ21WA
    TemperatureSensorEnable    // DS18B20
};

enum HostPCCommands {
	PC_Command_NONE,
	PC_Command_START,
	PC_Command_RESET
};

struct CommMessage
{
	enum SensorId_t SensorID;
	uint8_t messageId;
	uint16_t params;
	uint8_t checksum;
	bool IsCheckSumValid;
	bool IsMessageReady; // This flag indicates if a message has been decoded by the datalink.
};

//Send data based on sensor type
void send_sensorData_message(enum SensorId_t sensorType, uint16_t data);

// acknowledge messages to confirm that command messages have been received.
void send_ack_message(enum AckTypes AckType);

void send_sensorEnable_message(enum SensorId_t sensorType, uint16_t TimePeriod);

void send_sensorReset_message(void);

void initialize_sensor_datalink(void);
void initialize_hostPC_datalink(void);
void parse_sensor_message(struct CommMessage* currentRxMessage);

enum HostPCCommands parse_hostPC_message();

#endif /* INC_USER_L2_COMM_DATALINK_H_ */
