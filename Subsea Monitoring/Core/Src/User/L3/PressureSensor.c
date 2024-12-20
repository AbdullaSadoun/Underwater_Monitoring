#include <stdlib.h>

#include "User/L2/Comm_Datalink.h"
#include "User/L3/PressureSensor.h"
#include "User/L2/DataPlotter.h"

// Required FreeRTOS header files
#include "FreeRTOS.h"
#include "Timers.h"

/******************************************************************************
This is a software callback function.
******************************************************************************/
void RunPressureSensor(TimerHandle_t xTimer) // Default 1000 ms
{
    const uint16_t variance = 50; // variance for simulated pressure data
    const uint16_t mean = 100; // mean for simulated pressure data

    // Simulate pressure sensor data
    uint16_t simulatedPressure = (rand() % variance) + mean;

    // Send the simulated data
    send_sensorData_message(Pressure, simulatedPressure);
    send_plot_data(Pressure, simulatedPressure, xTaskGetTickCount());
}
