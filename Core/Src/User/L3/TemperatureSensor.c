#include <stdlib.h>

#include "User/L2/Comm_Datalink.h"
#include "User/L3/TemperatureSensor.h"
#include "User/L2/DataPlotter.h"

// Required FreeRTOS header files
#include "FreeRTOS.h"
#include "Timers.h"

/******************************************************************************
This is a software callback function.
******************************************************************************/
void RunTemperatureSensor(TimerHandle_t xTimer) // Default 1000 ms
{
    const uint16_t variance = 5; // Variance for simulated temperature data
    const uint16_t mean = 25; // Mean for simulated temperature data (DS18B20 typical range)

    // Simulate temperature sensor data
    uint16_t simulatedTemperature = (rand() % variance) + mean;

    // Send the simulated data
    send_sensorData_message(Temperature, simulatedTemperature);
    send_plot_data(Temperature, simulatedTemperature, xTaskGetTickCount());
} 