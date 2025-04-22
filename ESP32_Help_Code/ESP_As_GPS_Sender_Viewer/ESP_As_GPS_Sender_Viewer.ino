#include <BluetoothSerial.h>

#define RXD2 16  // ESP32 UART2 RX
#define TXD2 17  // ESP32 UART2 TX

BluetoothSerial SerialBT;  // Create BluetoothSerial object

void Send_GPS_Data();

void setup() {
  
  // For acting as module
  Serial.begin(9600);                             // USB Serial
  
  // For receiving output
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);    // UART2
  
  // for viewing the output
  SerialBT.begin("ESP32_BT");                     // Bluetooth device name

}

void loop() {
  
  Send_GPS_Data();

  if (Serial2.available()) {
    String msg = Serial2.readStringUntil('\n');
    SerialBT.println(msg); 
  }

}

void Send_GPS_Data(){
  Serial.println("$GPGGA,123519.00,3749.1234,N,12225.6789,W,1,08,0.9,545.4,M,46.9,M,,*47");
  Serial.println("$GPRMC,123519.00,A,3749.1234,N,12225.6789,W,0.13,309.62,210425,,,A*77");
  Serial.println("$GPGSA,A,3,04,05,09,12,24,25,29,31,32,,,1.8,1.0,1.5*33");
  Serial.println("$GPGSV,3,1,12,04,77,048,42,05,63,123,43,09,58,234,42,12,45,067,43*70");
  Serial.println("$GPGSV,3,2,12,24,38,312,41,25,28,157,40,29,17,245,40,31,12,086,39*7B");
  Serial.println("$GPGSV,3,3,12,32,07,295,38,39,05,123,00,48,03,273,00*40");
  /*
    Explanation of Key Sentences
      $GPGGA — Global Positioning System Fix Data
      Time: 12:35:19 UTC
      Latitude: 37°49.1234' N
      Longitude: 122°25.6789' W
      Fix quality: 1 (GPS fix)
      Satellites used: 8
      Altitude: 545.4 meters

      $GPRMC — Recommended Minimum Specific GPS/Transit Data
      Time: 12:35:19 UTC
      Status: A (active)
      Latitude/Longitude as above
      Speed over ground: 0.13 knots
      Track angle: 309.62°
      Date: 21 April 2025
      
      $GPGSA — GPS DOP and active satellites
      $GPGSV — Satellites in view (3 messages for 12 satellites)
      $GPVTG — Track made good and ground speed
  */
}
