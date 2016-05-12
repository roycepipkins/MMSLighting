#include <Arduino.h>

#include <inttypes.h>
#include <ESP8266WiFi.h>
#include <MCP23S17.h>
#include "MCP_IO.h"
#include "LightZone.h"
#include "Timestamp.h"

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define AT_HOME
#include "Credentials.h"

const int Wifi_LED = 5;
const int Mqtt_LED = 4;

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char Mqtt_Server[] PROGMEM    = MQTT_SERVER;
const char Mqtt_Username[] PROGMEM  = MQTT_USERNAME;
const char Mqtt_Password[] PROGMEM  = MQTT_KEY;

//Cmd will ON/OFF
//Sts will be ON, OVERRIDDEN, PENDING_OFF, OFF

const char LZ1_Cmd_Topic[] PROGMEM = "Lighting/LZ1_Cmd";
const char LZ2_Cmd_Topic[] PROGMEM = "Lighting/LZ2_Cmd";
const char LZ3_Cmd_Topic[] PROGMEM = "Lighting/LZ3_Cmd";
const char LZ4_Cmd_Topic[] PROGMEM = "Lighting/LZ4_Cmd";
const char LZ5_Cmd_Topic[] PROGMEM = "Lighting/LZ5_Cmd";
const char LZ6_Cmd_Topic[] PROGMEM = "Lighting/LZ6_Cmd";
const char LZ7_Cmd_Topic[] PROGMEM = "Lighting/LZ7_Cmd";
const char LZ8_Cmd_Topic[] PROGMEM = "Lighting/LZ8_Cmd";

const char LZ1_Sts_Topic[] PROGMEM = "Lighting/LZ1_Sts";
const char LZ2_Sts_Topic[] PROGMEM = "Lighting/LZ2_Sts";
const char LZ3_Sts_Topic[] PROGMEM = "Lighting/LZ3_Sts";
const char LZ4_Sts_Topic[] PROGMEM = "Lighting/LZ4_Sts";
const char LZ5_Sts_Topic[] PROGMEM = "Lighting/LZ5_Sts";
const char LZ6_Sts_Topic[] PROGMEM = "Lighting/LZ6_Sts";
const char LZ7_Sts_Topic[] PROGMEM = "Lighting/LZ7_Sts";
const char LZ8_Sts_Topic[] PROGMEM = "Lighting/LZ8_Sts";


WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, Mqtt_Server, MQTT_SERVERPORT, Mqtt_Username, Mqtt_Password);

Adafruit_MQTT_Publish LZ1_Sts = Adafruit_MQTT_Publish(&mqtt, LZ1_Sts_Topic);
Adafruit_MQTT_Publish LZ2_Sts = Adafruit_MQTT_Publish(&mqtt, LZ2_Sts_Topic);
Adafruit_MQTT_Publish LZ3_Sts = Adafruit_MQTT_Publish(&mqtt, LZ3_Sts_Topic);
Adafruit_MQTT_Publish LZ4_Sts = Adafruit_MQTT_Publish(&mqtt, LZ4_Sts_Topic);
Adafruit_MQTT_Publish LZ5_Sts = Adafruit_MQTT_Publish(&mqtt, LZ5_Sts_Topic);
Adafruit_MQTT_Publish LZ6_Sts = Adafruit_MQTT_Publish(&mqtt, LZ6_Sts_Topic);
Adafruit_MQTT_Publish LZ7_Sts = Adafruit_MQTT_Publish(&mqtt, LZ7_Sts_Topic);
Adafruit_MQTT_Publish LZ8_Sts = Adafruit_MQTT_Publish(&mqtt, LZ8_Sts_Topic);

Adafruit_MQTT_Publish* Zone_Status_Pubs[] = {&LZ1_Sts, &LZ2_Sts, &LZ3_Sts, &LZ4_Sts, &LZ5_Sts, &LZ6_Sts, &LZ7_Sts, &LZ8_Sts};

Adafruit_MQTT_Subscribe LZ1_Cmd = Adafruit_MQTT_Subscribe(&mqtt, LZ1_Cmd_Topic);
Adafruit_MQTT_Subscribe LZ2_Cmd = Adafruit_MQTT_Subscribe(&mqtt, LZ2_Cmd_Topic);
Adafruit_MQTT_Subscribe LZ3_Cmd = Adafruit_MQTT_Subscribe(&mqtt, LZ3_Cmd_Topic);
Adafruit_MQTT_Subscribe LZ4_Cmd = Adafruit_MQTT_Subscribe(&mqtt, LZ4_Cmd_Topic);
Adafruit_MQTT_Subscribe LZ5_Cmd = Adafruit_MQTT_Subscribe(&mqtt, LZ5_Cmd_Topic);
Adafruit_MQTT_Subscribe LZ6_Cmd = Adafruit_MQTT_Subscribe(&mqtt, LZ6_Cmd_Topic);
Adafruit_MQTT_Subscribe LZ7_Cmd = Adafruit_MQTT_Subscribe(&mqtt, LZ7_Cmd_Topic);
Adafruit_MQTT_Subscribe LZ8_Cmd = Adafruit_MQTT_Subscribe(&mqtt, LZ8_Cmd_Topic);

Adafruit_MQTT_Subscribe* Zone_Cmd_Subs[] = {&LZ1_Cmd, &LZ2_Cmd, &LZ3_Cmd, &LZ4_Cmd, &LZ5_Cmd, &LZ6_Cmd, &LZ7_Cmd, &LZ8_Cmd};

MCP ssrs(0, 15);
MCP buttons(1, 15);
MCP_IO mcp_io(&ssrs, &buttons);

LightZone lz1(mcp_io, 0, 8,  24, 16);
LightZone lz2(mcp_io, 1, 9,  25, 17);
LightZone lz3(mcp_io, 2, 10, 26, 18);
LightZone lz4(mcp_io, 3, 11, 27, 19);
LightZone lz5(mcp_io, 4, 12, 28, 20);
LightZone lz6(mcp_io, 5, 13, 29, 21);
LightZone lz7(mcp_io, 6, 14, 30, 22);
LightZone lz8(mcp_io, 7, 15, 31, 23);

LightZone* lz[8] = {&lz1, &lz2, &lz3, &lz4, &lz5, &lz6, &lz7, &lz8};

int strncasecmp(const char* s1, const char* s2, int len)
{
  int i;
  for(i = 0; i < len && s1[i] && s2[i]; ++i)
  {
    if ((s1[i] | 32) != (s2[i] | 32))
    {
      return 1;
    }
  }

  return (!s1[i] && !s2[i] ? 0 : 1);
}

void setup() {

  Serial.begin(115200);
  delay(10);

  Serial.println("");
  Serial.println("");
  Serial.println("----------------------");
  Serial.println("MMS Master Light Panel");
  Serial.println("         v1.0         ");
  Serial.println("----------------------");

  for(int i = 0; i < 8; ++i)
  {
    lz[i]->Setup();
  }

  pinMode(Wifi_LED, OUTPUT);
  pinMode(Mqtt_LED, OUTPUT);
  digitalWrite(Wifi_LED, LOW);
  digitalWrite(Mqtt_LED, LOW);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("WiFi is connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);

  mqtt.subscribe(&LZ1_Cmd);
  mqtt.subscribe(&LZ2_Cmd);
  mqtt.subscribe(&LZ3_Cmd);
  mqtt.subscribe(&LZ4_Cmd);
  mqtt.subscribe(&LZ5_Cmd);
  mqtt.subscribe(&LZ6_Cmd);
  mqtt.subscribe(&LZ7_Cmd);
  mqtt.subscribe(&LZ8_Cmd);

  for(int i = 24; i < 32; ++i)
  {
    mcp_io.inputInvert(i, 1);
    
  }
}

bool IsMQTTConnected()
{
  static Timestamp last_ping;
  
  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(Wifi_LED, HIGH);
    if (mqtt.connected()) 
    {
      digitalWrite(Mqtt_LED, HIGH);
      if (last_ping.Elapsed() >= 30000)
      {
        last_ping.Update();
        if(!mqtt.ping())
        {
          mqtt.disconnect();
          digitalWrite(Mqtt_LED, LOW);
        }
      }
      return true;
    }
    else
    {
      digitalWrite(Mqtt_LED, LOW);
      mqtt.disconnect();
      if (mqtt.connect() == 0) 
      {
        Serial.println("MQTT Connected.");
        digitalWrite(Mqtt_LED, HIGH);
        return true;
      }
    }
  }
  else
  {
    digitalWrite(Wifi_LED, LOW);
    digitalWrite(Mqtt_LED, LOW);
    mqtt.disconnect();
  }

  return false;
}

void PrintState(const int i, const char* state)
{
  Serial.print("Zone #"),
  Serial.print(i, DEC);
  Serial.print(" state is now: ");
  Serial.println(state);
}

void loop()
{
  if (IsMQTTConnected())
  {
    Adafruit_MQTT_Subscribe *subscription = 0;

    while (subscription = mqtt.readSubscription(10))
    {
      for(int i = 0; i < 8; ++i)
      {
        if (subscription == Zone_Cmd_Subs[i])
        {
          if (strncasecmp("On", (char*)(subscription->lastread), 2) == 0)
          {
            if (lz[i]->TurnOn())
            {
              //Zone_Status_Pubs[i]->publish(lz[i]->GetStatusText());
              //PrintState(i, lz[i]->GetStatusText());
            }
          }
          if (strncasecmp("Off", (char*)(subscription->lastread), 3) == 0)
          {
            if (lz[i]->StartPendingOff())
            {
              //Zone_Status_Pubs[i]->publish(lz[i]->GetStatusText());
              //PrintState(i, lz[i]->GetStatusText());
            }
          }
          break;
        }
      }
    }
  }

  for(int i = 0; i < 8; ++i)
  {
    if (lz[i]->Update())
    {
        Zone_Status_Pubs[i]->publish(lz[i]->GetStatusText());
        PrintState(i, lz[i]->GetStatusText());
    }
  }
}
