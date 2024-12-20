#ifndef INC_USER_L3_FLOWRATESENSOR_H_
#define INC_USER_L3_FLOWRATESENSOR_H_

#include "FreeRTOS.h"
#include "timers.h"

void RunFlowRateSensor(TimerHandle_t xTimer);

#endif /* INC_USER_L3_FLOWRATESENSOR_H_ */
