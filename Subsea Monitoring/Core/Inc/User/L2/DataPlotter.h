#ifndef INC_USER_L2_DATAPLOTTER_H_
#define INC_USER_L2_DATAPLOTTER_H_

#include "User/L2/Comm_Datalink.h"

void send_plot_data(enum SensorId_t sensorType, uint16_t data, uint32_t timestamp);
void initialize_plotter(void);

#endif /* INC_USER_L2_DATAPLOTTER_H_ */ 