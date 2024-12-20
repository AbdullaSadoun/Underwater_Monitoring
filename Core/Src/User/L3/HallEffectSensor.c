#include <stdlib.h>

#include "User/L2/Comm_Datalink.h"
#include "User/L3/HallEffectSensor.h"
#include "User/L2/DataPlotter.h"

// Required FreeRTOS header files
#include "FreeRTOS.h"
#include "Timers.h"

/******************************************************************************
This is a software callback function.
******************************************************************************/
void RunHallEffectSensor(TimerHandle_t xTimer) // Default 1000 ms
{
    const uint16_t variance = 25; // Variance for simulated hall effect data
    const uint16_t mean = 128; // Mean for simulated hall effect data (SEN-HZ21WA range)

    // Simulate hall effect sensor data
    uint16_t simulatedHallEffect = (rand() % variance) + mean;

    // Send the simulated data
    send_sensorData_message(HallEffect, simulatedHallEffect);
    send_plot_data(HallEffect, simulatedHallEffect, xTaskGetTickCount());
} 