/*
 * util.c
 *
 *  Created on: Aug 8, 2022
 *      Author: Andre Hendricks
 */
#include "main.h"
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"

static SemaphoreHandle_t mutexHandle_print_str;
extern UART_HandleTypeDef huart2;

void util_init(){
	mutexHandle_print_str = xSemaphoreCreateMutex();
}

static void print_str_local(char * str){
	HAL_UART_Transmit(&huart2,(uint8_t*) str, strlen(str), HAL_MAX_DELAY);
}


void print_str(char * str){
	xSemaphoreTake(mutexHandle_print_str, portMAX_DELAY);
	print_str_local(str);
	xSemaphoreGive(mutexHandle_print_str);
}
void print_str_ISR(char * str){
	print_str_local(str);
}

void print_str_unsafe(char * str){
	for(int i =0; i<strlen(str);i++){
		HAL_UART_Transmit(&huart2,(uint8_t*) &str[i], 1, HAL_MAX_DELAY);
		for(int j=0;j<100000;j++);
	}
}
