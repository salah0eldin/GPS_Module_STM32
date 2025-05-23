/*
 * gps_neo_7_hal.c
 *
 *  Created on: Apr 22, 2025
 *      Author: salah0eldin
 */

// ====================================================
// Includes and Definitions
// ====================================================
#include "gps_neo_7_hal.h"

#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

UART_HandleTypeDef *huart;

// #define huart huart1
#define TRIES 400

static char raw_buffer[GPS_BUFFER_SIZE];

// ====================================================
// Function Prototypes
// ====================================================
static int decodeGGA(char *GGAbuffer, GGASTRUCT *gga);
static int decodeRMC(char *RMCbuffer, RMCSTRUCT *rmc);

// ====================================================
// GPS HAL Initialization
// ====================================================
void GPS_HAL_Init(UART_HandleTypeDef *h)
{
    // Initialize the UART handle for GPS communication
    // Disable USART1 interrupt (if previously enabled)
    huart = h;
}

// ====================================================
// GPS HAL Receive Raw GGA Data
// ====================================================
HAL_StatusTypeDef GPS_HAL_Receive_Raw_GGA_Data(char *buffer)
{
    char data = 0;
    int i = TRIES;

    while (i > 0)
    {
        // Receive one byte of data
        if (HAL_UART_Receive(huart, (uint8_t *)&data, 1, HAL_MAX_DELAY) != HAL_OK)
            return HAL_TIMEOUT; // Error in receiving data

        // Check for the start of GGA sentence ('G')
        if (data == 'G')
        {
            if (HAL_UART_Receive(huart, (uint8_t *)&data, 1, HAL_MAX_DELAY) != HAL_OK)
                return HAL_TIMEOUT;

            // Check for 'A' or another 'G' to confirm GGA sentence
            if (data == 'A' || data == 'G')
            {
                // Receive the rest of the GGA sentence
                if (HAL_UART_Receive(huart, (uint8_t *)buffer, GPS_BUFFER_SIZE, HAL_MAX_DELAY) != HAL_OK)
                    return HAL_TIMEOUT;

                return HAL_OK; // Successfully received GGA data
            }
        }

        i--; // Decrement retry counter
    }

    return HAL_TIMEOUT; // Timeout or no valid data received
}

// ====================================================
// GPS HAL Receive Raw RMC Data
// ====================================================
HAL_StatusTypeDef GPS_HAL_Receive_Raw_RMC_Data(char *buffer)
{
    char data = 0;
    int i = TRIES;

    while (i > 0)
    {
        // Receive one byte of data
        if (HAL_UART_Receive(huart, (uint8_t *)&data, 1, HAL_MAX_DELAY) != HAL_OK)
            return HAL_TIMEOUT; // Error in receiving data

        // Check for the start of RMC sentence ('M')
        if (data == 'M')
        {
            if (HAL_UART_Receive(huart, (uint8_t *)&data, 1, HAL_MAX_DELAY) != HAL_OK)
                return HAL_TIMEOUT;

            // Check for 'C' to confirm RMC sentence
            if (data == 'C')
            {
                // Receive the rest of the RMC sentence
                if (HAL_UART_Receive(huart, (uint8_t *)buffer, GPS_BUFFER_SIZE, HAL_MAX_DELAY) != HAL_OK)
                    return HAL_TIMEOUT;

                return HAL_OK; // Successfully received RMC data
            }
        }

        i--; // Decrement retry counter
    }

    return HAL_TIMEOUT; // Timeout or no valid data received
}

// ====================================================
// GPS HAL Receive and Decode Data
// ====================================================
void GPS_HAL_Receive_Decode_Data(GPS_DATA *data)
{
    // Attempt to receive and decode GGA data
    if (GPS_HAL_Receive_Raw_GGA_Data(&raw_buffer) == HAL_OK)
        decodeGGA(raw_buffer, &data->ggastruct);

    // Attempt to receive and decode RMC data
    if (GPS_HAL_Receive_Raw_RMC_Data(&raw_buffer) == HAL_OK)
        decodeRMC(raw_buffer, &data->rmcstruct);
}

// ====================================================
// Static Variables for Decoding
// ====================================================
static int GMT = GMT_OFFSET;
static int inx = 0;
static int hr = 0, min = 0, day = 0, mon = 0, yr = 0;
static int daychange = 0;

// ====================================================
// Decode GGA Data
// ====================================================
/* Decodes the GGA Data
   @GGAbuffer is the buffer which stores the GGA Data
   @GGASTRUCT is the pointer to the GGA Structure (in the GPS Structure)
   @Returns 0 on success
   @Returns 1, 2 depending on where the return statement is executed, check function for more details
*/
static int decodeGGA(char *GGAbuffer, GGASTRUCT *gga)
{
    inx = 0;
    char buffer[12];
    int i = 0;

    // Ensure inx does not exceed buffer size
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE)
        inx++; // 1st ','
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    inx++;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE)
        inx++; // After time ','
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    inx++;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE)
        inx++; // after latitude ','
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    inx++;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE)
        inx++; // after NS ','
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    inx++;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE)
        inx++; // after longitude ','
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    inx++;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE)
        inx++; // after EW ','
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    inx++; // reached the character to identify the fix
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    if ((GGAbuffer[inx] == '1') || (GGAbuffer[inx] == '2') || (GGAbuffer[inx] == '6')) // 0 indicates no fix yet
    {
        gga->isfixValid = 1; // fix available
        inx = 0;             // reset the index. We will start from the inx=0 and extract information now
    }
    else
    {
        gga->isfixValid = 0; // If the fix is not available
        return 1;            // return error
    }
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE)
        inx++; // 1st ','

    /*********************** Get TIME ***************************/
    //(Update the GMT Offset at the top of this file)

    inx++; // reach the first number in time
    memset(buffer, '\0', 12);
    i = 0;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE) // copy upto the we reach the after time ','
    {
        buffer[i] = GGAbuffer[inx];
        i++;
        inx++;
    }
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    hr = (atoi(buffer) / 10000) + GMT / 100; // get the hours from the 6 digit number

    min = ((atoi(buffer) / 100) % 100) + GMT % 100; // get the minutes from the 6 digit number

    // adjust time.. This part still needs to be tested
    if (min > 59)
    {
        min = min - 60;
        hr++;
    }
    if (hr < 0)
    {
        hr = 24 + hr;
        daychange--;
    }
    if (hr >= 24)
    {
        hr = hr - 24;
        daychange++;
    }

    // Store the time in the GGA structure
    gga->tim.hour = hr;
    gga->tim.min = min;
    gga->tim.sec = atoi(buffer) % 100;

    /***************** Get LATITUDE  **********************/
    inx++; // Reach the first number in the lattitude
    memset(buffer, '\0', 12);
    i = 0;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE) // copy upto the we reach the after lattitude ','
    {
        buffer[i] = GGAbuffer[inx];
        i++;
        inx++;
    }
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    if (strlen(buffer) < 6)
        return 2;                 // If the buffer length is not appropriate, return error
    int16_t num = (atoi(buffer)); // change the buffer to the number. It will only convert upto decimal
    int j = 0;
    while (buffer[j] != '.')
        j++; // Figure out how many digits before the decimal
    j++;
    int declen = (strlen(buffer)) - j;                         // calculate the number of digit after decimal
    int dec = atoi((char *)buffer + j);                        // conver the decimal part a a separate number
    float lat = (num / 100.0) + (dec / pow(10, (declen + 2))); // 1234.56789 = 12.3456789
    gga->lcation.latitude = lat;                               // save the lattitude data into the strucure
    inx++;
    gga->lcation.NS = GGAbuffer[inx]; // save the N/S into the structure

    /***********************  GET LONGITUDE **********************/
    inx++; // ',' after NS character
    inx++; // Reach the first number in the longitude
    memset(buffer, '\0', 12);
    i = 0;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE) // copy upto the we reach the after longitude ','
    {
        buffer[i] = GGAbuffer[inx];
        i++;
        inx++;
    }
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    num = (atoi(buffer)); // change the buffer to the number. It will only convert upto decimal
    j = 0;
    while (buffer[j] != '.')
        j++; // Figure out how many digits before the decimal
    j++;
    declen = (strlen(buffer)) - j;                       // calculate the number of digit after decimal
    dec = atoi((char *)buffer + j);                      // conver the decimal part a a separate number
    lat = (num / 100.0) + (dec / pow(10, (declen + 2))); // 1234.56789 = 12.3456789
    gga->lcation.longitude = lat;                        // save the longitude data into the strucure
    inx++;
    gga->lcation.EW = GGAbuffer[inx]; // save the E/W into the structure

    /**************************************************/
    // skip positition fix
    inx++; // ',' after E/W
    inx++; // position fix
    inx++; // ',' after position fix;

    // number of sattelites
    inx++; // Reach the first number in the satellites
    memset(buffer, '\0', 12);
    i = 0;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE) // copy upto the ',' after number of satellites
    {
        buffer[i] = GGAbuffer[inx];
        i++;
        inx++;
    }
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    gga->numofsat = atoi(buffer); // convert the buffer to number and save into the structure

    /***************** skip HDOP  *********************/
    inx++;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE)
        inx++;
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    /*************** Altitude calculation ********************/
    inx++;
    memset(buffer, '\0', 12);
    i = 0;
    while (GGAbuffer[inx] != ',' && inx < GPS_BUFFER_SIZE)
    {
        buffer[i] = GGAbuffer[inx];
        i++;
        inx++;
    }
    if (inx >= GPS_BUFFER_SIZE) return 1; // Error: Index out of bounds

    num = (atoi(buffer));
    j = 0;
    while (buffer[j] != '.')
        j++;
    j++;
    declen = (strlen(buffer)) - j;
    dec = atoi((char *)buffer + j);
    lat = (num) + (dec / pow(10, (declen)));
    gga->alt.altitude = lat;

    inx++;
    gga->alt.unit = GGAbuffer[inx];

    return 0;
}

// ====================================================
// Decode RMC Data
// ====================================================
/* Decodes the RMC Data
   @RMCbuffer is the buffer which stores the RMC Data
   @RMCSTRUCT is the pointer to the RMC Structure (in the GPS Structure)
   @Returns 0 on success
   @Returns 1 if data is invalid
*/
static int decodeRMC(char *RMCbuffer, RMCSTRUCT *rmc)
{
    inx = 0;
    char buffer[12];
    int i = 0;
    while (RMCbuffer[inx] != ',')
        inx++; // 1st ,
    inx++;
    while (RMCbuffer[inx] != ',')
        inx++; // After time ,
    inx++;
    if (RMCbuffer[inx] == 'A') // Here 'A' Indicates the data is valid, and 'V' indicates invalid data
    {
        rmc->isValid = 1;
    }
    else
    {
        rmc->isValid = 0;
        return 1;
    }
    inx++;
    inx++;
    while (RMCbuffer[inx] != ',')
        inx++; // after latitude,
    inx++;
    while (RMCbuffer[inx] != ',')
        inx++; // after NS ,
    inx++;
    while (RMCbuffer[inx] != ',')
        inx++; // after longitude ,
    inx++;
    while (RMCbuffer[inx] != ',')
        inx++; // after EW ,

    // Get Speed
    inx++;
    i = 0;
    memset(buffer, '\0', 12);
    while (RMCbuffer[inx] != ',')
    {
        buffer[i] = RMCbuffer[inx];
        i++;
        inx++;
    }

    if (strlen(buffer) > 0)
    {                                 // if the speed have some data
        int16_t num = (atoi(buffer)); // convert the data into the number
        int j = 0;
        while (buffer[j] != '.')
            j++; // same as above
        j++;
        int declen = (strlen(buffer)) - j;
        int dec = atoi((char *)buffer + j);
        float lat = num + (dec / pow(10, (declen)));
        rmc->speed = lat;
    }
    else
        rmc->speed = 0;

    // Get Course
    inx++;
    i = 0;
    memset(buffer, '\0', 12);
    while (RMCbuffer[inx] != ',')
    {
        buffer[i] = RMCbuffer[inx];
        i++;
        inx++;
    }

    if (strlen(buffer) > 0)
    {                                 // if the course have some data
        int16_t num = (atoi(buffer)); // convert the course data into the number
        int j = 0;
        while (buffer[j] != '.')
            j++; // same as above
        j++;
        int declen = (strlen(buffer)) - j;
        int dec = atoi((char *)buffer + j);
        float lat = num + (dec / pow(10, (declen)));
        rmc->course = lat;
    }
    else
    {
        rmc->course = 0;
    }

    // Get Date
    inx++;
    i = 0;
    memset(buffer, '\0', 12);
    while (RMCbuffer[inx] != ',')
    {
        buffer[i] = RMCbuffer[inx];
        i++;
        inx++;
    }

    // Date in the format 280222
    day = atoi(buffer) / 10000;       // extract 28
    mon = (atoi(buffer) / 100) % 100; // extract 02
    yr = atoi(buffer) % 100;          // extract 22

    day = day + daychange; // correction due to GMT shift

    // save the data into the structure
    rmc->date.Day = day;
    rmc->date.Mon = mon;
    rmc->date.Yr = yr;

    return 0;
}
