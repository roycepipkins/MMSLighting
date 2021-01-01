#include <Arduino.h>
#include <SPI.h>
#include <inttypes.h>
#include <MCP23S17.h>

#include "MCP_IO.h"
#include <esp_wifi.h>
#include "Timestamp.h"


#define DEMO_MAKERSPACE
#include "Credentials.h"






const int Wifi_LED = 33;
const int Mqtt_LED = 32;





SPIClass spi(HSPI);
const int slaveSelect = 14;
MCP_IO mcp_io(spi, slaveSelect);








void setup() {
  esp_wifi_deinit();
  Serial.begin(115200);
  delay(10);

  Serial.println("");
  Serial.println("");
  Serial.println("----------------------");
  Serial.println("       GPIO Test      ");
  Serial.println("         v3.0         ");
  Serial.println("----------------------");


  pinMode(Wifi_LED, OUTPUT);
  pinMode(Mqtt_LED, OUTPUT);
  digitalWrite(Wifi_LED, HIGH);
  digitalWrite(Mqtt_LED, HIGH);

  pinMode(slaveSelect, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(13, OUTPUT);
  //pinMode(4, INPUT);

  spi.begin(13, 4, 3);
  spi.setFrequency(10000);
  //spi.setBitOrder(MSBFIRST);          // Sets SPI bus bit order (this is the default, setting it for good form!)
  //spi.setDataMode(SPI_MODE0); 

  for(int i = 0; i < 16; ++i)
  {
    mcp_io.pinMode(i, OUTPUT);
  }
  
}



void loop()
{
  /*digitalWrite(slaveSelect, HIGH);
  digitalWrite(3, HIGH);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(slaveSelect, LOW);
  digitalWrite(3, LOW);
  digitalWrite(13, LOW);
  delay(500);
  */

  
  
  for(int i = 0; i < 16; ++i)
  {
    delay(10);
    Serial.print("GPIO: ");
    Serial.println(i);
    mcp_io.digitalWrite(i, HIGH);
    delay(10);
    mcp_io.digitalWrite(i, LOW);
  }
  
}
