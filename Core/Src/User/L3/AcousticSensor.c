#include <stdlib.h>

#include "User/L2/Comm_Datalink.h"
#include "User/L3/AcousticSensor.h"
#include "User/L2/DataPlotter.h"

// Required FreeRTOS header files
#include "FreeRTOS.h"
#include "Timers.h"

/******************************************************************************
This is a software callback function.
******************************************************************************/
void RunAcousticSensor(TimerHandle_t xTimer) // Default 1000 ms
{
    char str[60];
    sprintf(str, "Acoustic sensor callback executing\r\n");
    print_str(str);
    
    const uint16_t variance = 20; // Variance for simulated acoustic data
    const uint16_t mean = 75; // Mean for simulated acoustic data

    // Simulate acoustic sensor data
    uint16_t simulatedAcousticData = (rand() % variance) + mean;

    // Send the simulated data
    send_sensorData_message(Acoustic, simulatedAcousticData);
    send_plot_data(Acoustic, simulatedAcousticData, xTaskGetTickCount());
}
