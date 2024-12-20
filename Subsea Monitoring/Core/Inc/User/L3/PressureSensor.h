#ifndef INC_USER_L3_PRESSURESENSOR_H_
#define INC_USER_L3_PRESSURESENSOR_H_

#include "FreeRTOS.h"
#include "timers.h"

void RunPressureSensor(TimerHandle_t xTimer);

#endif /* INC_USER_L3_PRESSURESENSOR_H_ */
