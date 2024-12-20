#include <stdlib.h>

#include "User/L2/Comm_Datalink.h"
#include "User/L3/Acoustic2Sensor.h"
#include "User/L2/DataPlotter.h"

// Required FreeRTOS header files
#include "FreeRTOS.h"
#include "Timers.h"

/******************************************************************************
This is a software callback function.
******************************************************************************/
void RunAcoustic2Sensor(TimerHandle_t xTimer) // Default 1000 ms
{
    const uint16_t variance = 15; // Variance for simulated acoustic data
    const uint16_t mean = 65; // Mean for simulated acoustic data (HTI-96MIN typical range)

    // Simulate acoustic sensor data
    uint16_t simulatedAcousticData = (rand() % variance) + mean;

    // Send the simulated data
    send_sensorData_message(Acoustic, simulatedAcousticData);
    send_plot_data(Acoustic, simulatedAcousticData, xTaskGetTickCount());
} 
