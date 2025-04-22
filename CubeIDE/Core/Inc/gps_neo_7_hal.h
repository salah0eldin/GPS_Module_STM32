/*
 * gps_neo_7_hal.h
 *
 *  Created on: Apr 22, 2025
 *      Author: salah0eldin
 */

#ifndef SRC_GPS_NEO_7_HAL_H_
#define SRC_GPS_NEO_7_HAL_H_

#define GPS_BUFFER_SIZE 100

#include "stm32f1xx_hal.h"

typedef struct {
	int hour;
	int min;
	int sec;
}TIME;

typedef struct {
	float latitude;
	char NS;
	float longitude;
	char EW;
}LOCATION;

typedef struct {
	float altitude;
	char unit;
}ALTITUDE;

typedef struct {
	int Day;
	int Mon;
	int Yr;
}DATE;

typedef struct {
	LOCATION lcation;
	TIME tim;
	int isfixValid;
	ALTITUDE alt;
	int numofsat;
}GGASTRUCT;

typedef struct {
	DATE date;
	float speed;
	float course;
	int isValid;
}RMCSTRUCT;

typedef struct {
	GGASTRUCT ggastruct;
	RMCSTRUCT rmcstruct;
}GPS_DATA;

void GPS_HAL_Init(UART_HandleTypeDef * h);

HAL_StatusTypeDef GPS_HAL_Receive_Raw_GGA_Data(char * buffer);

HAL_StatusTypeDef GPS_HAL_Receive_Raw_RMC_Data(char * buffer);

void GPS_HAL_Receive_Decode_Data(GPS_DATA * data);

#endif /* SRC_GPS_NEO_7_HAL_H_ */
