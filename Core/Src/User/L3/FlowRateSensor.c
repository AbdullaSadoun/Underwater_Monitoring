#include <stdlib.h>

#include "User/L2/Comm_Datalink.h"
#include "User/L3/FlowRateSensor.h"
#include "User/L2/DataPlotter.h"

// Required FreeRTOS header files
#include "FreeRTOS.h"
#include "Timers.h"

/******************************************************************************
This is a software callback function.
******************************************************************************/
void RunFlowRateSensor(TimerHandle_t xTimer) // Default 1000 ms
{
    const uint16_t variance = 30; // Variance for simulated flow rate
    const uint16_t mean = 200; // Mean for simulated flow rate

    // Simulate flow rate sensor data
    uint16_t simulatedFlowRate = (rand() % variance) + mean;

    // Send the simulated data
    send_sensorData_message(Acoustic, simulatedFlowRate);
    send_plot_data(Acoustic, simulatedFlowRate, xTaskGetTickCount());
}
