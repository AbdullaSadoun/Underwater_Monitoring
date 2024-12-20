#include <stdlib.h>

#include "User/L2/Comm_Datalink.h"
#include "User/L3/CorrosionSensor.h"
#include "User/L2/DataPlotter.h"

// Required FreeRTOS header files
#include "FreeRTOS.h"
#include "Timers.h"

/******************************************************************************
This is a software callback function.
******************************************************************************/
void RunCorrosionSensor(TimerHandle_t xTimer) // Default 1000 ms
{
    const uint16_t variance = 10; // Variance for simulated corrosion rate
    const uint16_t mean = 50; // Mean for simulated corrosion rate

    // Simulate corrosion sensor data
    uint16_t simulatedCorrosionRate = (rand() % variance) + mean;

    // Send the simulated data
    //send_sensorData_message(Corrosion, simulatedCorrosionRate);
}
