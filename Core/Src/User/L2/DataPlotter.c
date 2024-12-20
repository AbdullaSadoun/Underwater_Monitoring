#include <stdio.h>
#include "User/L2/DataPlotter.h"
#include "User/L2/Comm_Datalink.h"
#include "User/util.h"

void send_plot_data(enum SensorId_t sensorType, uint16_t data, uint32_t timestamp) {
    char plot_buffer[100];
    
    // Format: @PLOT,sensor_id,timestamp,value\n
    sprintf(plot_buffer, "@PLOT,%d,%lu,%u\n", sensorType, timestamp, data);
    print_str(plot_buffer);
}

void initialize_plotter(void) {
    print_str("@PLOT_INIT\n");
} 