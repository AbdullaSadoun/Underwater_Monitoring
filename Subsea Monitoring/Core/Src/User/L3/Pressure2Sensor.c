#include <stdlib.h>

#include "User/L2/Comm_Datalink.h"
#include "User/L3/Pressure2Sensor.h"
#include "User/L2/DataPlotter.h"

// Required FreeRTOS header files
#include "FreeRTOS.h"
#include "Timers.h"

/******************************************************************************
This is a software callback function.
******************************************************************************/
void RunPressure2Sensor(TimerHandle_t xTimer) // Default 1000 ms
{
    const uint16_t variance = 40; // Variance for simulated pressure data
    const uint16_t mean = 150; // Mean for simulated pressure data (MS5837-30BA range)

    // Simulate pressure sensor data
    uint16_t simulatedPressure = (rand() % variance) + mean;

    // Send the simulated data
    send_sensorData_message(Acoustic, simulatedPressure);
    send_plot_data(Acoustic, simulatedPressure, xTaskGetTickCount());
} 
