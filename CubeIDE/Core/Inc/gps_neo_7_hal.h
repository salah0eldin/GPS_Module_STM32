/*
 * gps_neo_7_hal.h
 *
 *  Created on: Apr 22, 2025
 *      Author: salah0eldin
 *
 *  Description:
 *  This header file provides the interface for interacting with the NEO-7 GPS module
 *  using STM32 HAL. It defines data structures for GPS data (GGA and RMC), as well as
 *  function prototypes for initializing the GPS module, receiving raw data, and decoding
 *  GPS information.
 */

#ifndef SRC_GPS_NEO_7_HAL_H_
#define SRC_GPS_NEO_7_HAL_H_

#include "stm32f1xx_hal.h"

// Buffer size for storing raw GPS data
#define GPS_BUFFER_SIZE 100

// Define the GMT offset for time conversion
#define GMT_OFFSET 0

// Structure to represent time (hours, minutes, seconds)
typedef struct {
    int hour;  // Hour (0-23)
    int min;   // Minute (0-59)
    int sec;   // Second (0-59)
} TIME;

// Structure to represent GPS location (latitude, longitude, and direction)
typedef struct {
    float latitude;     // Latitude in degrees
    char NS;            // North/South indicator ('N' or 'S')
    float longitude;    // Longitude in degrees
    char EW;            // East/West indicator ('E' or 'W')
} LOCATION;

// Structure to represent altitude information
typedef struct {
    float altitude; // Altitude in meters
    char unit;      // Unit of altitude ('M' for meters)
} ALTITUDE;

// Structure to represent date (day, month, year)
typedef struct {
    int Day; // Day of the month (1-31)
    int Mon; // Month (1-12)
    int Yr;  // Year (e.g., 2025)
} DATE;

// Structure to represent GGA data (location, time, altitude, etc.)
typedef struct {
    LOCATION lcation; // GPS location
    TIME tim;         // Time of fix
    int isfixValid;   // Fix validity (1 if valid, 0 otherwise)
    ALTITUDE alt;     // Altitude information
    int numofsat;     // Number of satellites used in the fix
} GGASTRUCT;

// Structure to represent RMC data (date, speed, course, etc.)
typedef struct {
    DATE date;       // Date of fix
    float speed;     // Speed over ground in knots
    float course;    // Course over ground in degrees
    int isValid;     // Validity of RMC data (1 if valid, 0 otherwise)
} RMCSTRUCT;

// Structure to represent all GPS data (GGA and RMC)
typedef struct {
    GGASTRUCT ggastruct; // GGA data
    RMCSTRUCT rmcstruct; // RMC data
} GPS_DATA;

// Function to initialize the GPS module with the specified UART handle
void GPS_HAL_Init(UART_HandleTypeDef * h);

// Function to receive raw GGA data from the GPS module
HAL_StatusTypeDef GPS_HAL_Receive_Raw_GGA_Data(char * buffer);

// Function to receive raw RMC data from the GPS module
HAL_StatusTypeDef GPS_HAL_Receive_Raw_RMC_Data(char * buffer);

// Function to decode raw GPS data into structured GPS_DATA
void GPS_HAL_Receive_Decode_Data(GPS_DATA * data);

#endif /* SRC_GPS_NEO_7_HAL_H_ */
