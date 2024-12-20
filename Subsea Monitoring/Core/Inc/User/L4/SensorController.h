/*
 * SensorController.h
 *
 *  Created on: Oct 24, 2022
 *      Author: kadh1
 */

#ifndef INC_USER_L4_SENSORCONTROLLER_H_
#define INC_USER_L4_SENSORCONTROLLER_H_


struct SensorStates {
    bool IsAcousticAck;
    bool IsPressureAck;     // Acknowledgment flag for Pressure Sensor
    bool IsFlowRateAck;     // Acknowledgment flag for Flow Rate Sensor
    bool IsCorrosionAck;    // Acknowledgment flag for Corrosion Sensor

    uint16_t AcousticData;  // Data from Acoustic Sensor
    uint16_t PressureData;  // Data from Pressure Sensor
    uint16_t FlowRateData;  // Data from Flow Rate Sensor
    uint16_t CorrosionData; // Data from Corrosion Sensor
};

void HostPC_RX_Task();
void SensorPlatform_RX_Task();
void SensorControllerTask(void *params);

char* analyzeAcousticValue(int acousticValue);
char* analyzePressureValue(int pressureValue);
char* analyzeFlowRateValue(int flowRateValue);
char* analyzeCorrosionValue(int corrosionValue);

// New Sensors

char* analyzeAcoustic2Value(int acousticValue);
char* analyzePressure2Value(int pressureValue);
char* analyzeHallEffectValue(int hallValue);
char* analyzeTemperatureValue(int tempValue);

#endif /* INC_USER_L4_SENSORCONTROLLER_H_ */
