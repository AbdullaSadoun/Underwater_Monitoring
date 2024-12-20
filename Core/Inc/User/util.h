/*
 * util.h
 *
 *  Created on: Aug 8, 2022
 *      Author: Andre Hendricks
 */

#ifndef INC_USER_UTIL_H_
#define INC_USER_UTIL_H_

void util_init();

void print_str(char * str);
void print_str_ISR(char * str);
void print_str_unsafe(char * str);

#endif /* INC_USER_UTIL_H_ */
