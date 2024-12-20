#ifndef INC_USER_L3_CORROSIONSENSOR_H_
#define INC_USER_L3_CORROSIONSENSOR_H_

#include "FreeRTOS.h"
#include "timers.h"

void RunCorrosionSensor(TimerHandle_t xTimer);

#endif /* INC_USER_L3_CORROSIONSENSOR_H_ */
