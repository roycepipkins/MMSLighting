#include <inttypes.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Timestamp.h"
#include "RGBConverter.h"
#include <Adafruit_NeoPixel.h>

#define AT_MAKERSPACE
#include "Credentials.h"

#define LIGHT_ZONE 1

const char Mqtt_Server[] PROGMEM    = MQTT_SERVER;
const char Mqtt_Username[] PROGMEM  = MQTT_USERNAME;
const char Mqtt_Password[] PROGMEM  = MQTT_KEY;



#if LIGHT_ZONE == 1
const char LZ_Timer_Topic[] PROGMEM = "Lighting/LZ1_Timer";
const char LZ_Sts_Topic[] PROGMEM = "Lighting/LZ1_Sts";
const char LZ_Cmd_Topic[] PROGMEM = "Lighting/LZ1_Cmd";
#elif LIGHT_ZONE == 2
const char LZ_Timer_Topic[] PROGMEM = "Lighting/LZ2_Timer";
const char LZ_Sts_Topic[] PROGMEM = "Lighting/LZ2_Sts";
const char LZ_Cmd_Topic[] PROGMEM = "Lighting/LZ2_Cmd";
#elif LIGHT_ZONE == 3
const char LZ_Timer_Topic[] PROGMEM = "Lighting/LZ3_Timer";
const char LZ_Sts_Topic[] PROGMEM = "Lighting/LZ3_Sts";
const char LZ_Cmd_Topic[] PROGMEM = "Lighting/LZ3_Cmd";
#elif LIGHT_ZONE == 4
const char LZ_Timer_Topic[] PROGMEM = "Lighting/LZ4_Timer";
const char LZ_Sts_Topic[] PROGMEM = "Lighting/LZ4_Sts";
const char LZ_Cmd_Topic[] PROGMEM = "Lighting/LZ4_Cmd";
#elif LIGHT_ZONE == 5
const char LZ_Timer_Topic[] PROGMEM = "Lighting/LZ5_Timer";
const char LZ_Sts_Topic[] PROGMEM = "Lighting/LZ5_Sts";
const char LZ_Cmd_Topic[] PROGMEM = "Lighting/LZ5_Cmd";
#elif LIGHT_ZONE == 6
const char LZ_Timer_Topic[] PROGMEM = "Lighting/LZ6_Timer";
const char LZ_Sts_Topic[] PROGMEM = "Lighting/LZ6_Sts";
const char LZ_Cmd_Topic[] PROGMEM = "Lighting/LZ6_Cmd";
#elif LIGHT_ZONE == 7
const char LZ_Timer_Topic[] PROGMEM = "Lighting/LZ7_Timer";
const char LZ_Sts_Topic[] PROGMEM = "Lighting/LZ7_Sts";
const char LZ_Cmd_Topic[] PROGMEM = "Lighting/LZ7_Cmd";
#elif LIGHT_ZONE == 8
const char LZ_Timer_Topic[] PROGMEM = "Lighting/LZ8_Timer";
const char LZ_Sts_Topic[] PROGMEM = "Lighting/LZ8_Sts";
const char LZ_Cmd_Topic[] PROGMEM = "Lighting/LZ8_Cmd";
#endif



const int LightSwitch_Pin = 5;
const int Wifi_LED = 12;
const int Mqtt_LED = 13; 
const int NeoPixel_Pin = 4;

const int Debounce_time = 75;

const int Max_Hue_Time = 180; //amount of time remaining for light to be green

//TODO set to a nice green
const double Max_Hue = 0.5; 
const double Saturation = 0.5;
const double Lightness = 0.5;

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, Mqtt_Server, MQTT_SERVERPORT, Mqtt_Username, Mqtt_Password);
Adafruit_MQTT_Publish LZ_Cmd = Adafruit_MQTT_Publish(&mqtt, LZ_Cmd_Topic);
Adafruit_MQTT_Subscribe LZ_Sts = Adafruit_MQTT_Subscribe(&mqtt, LZ_Sts_Topic);
Adafruit_MQTT_Subscribe LZ_Tmr = Adafruit_MQTT_Subscribe(&mqtt, LZ_Timer_Topic);

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, NeoPixel_Pin, NEO_GRB + NEO_KHZ800);

RGBConverter converter;




void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println("");
  Serial.println("");
  Serial.println("-------------------------");
  Serial.println("MMS Remote Light Switch  ");
  Serial.println("         v1.0            ");
  Serial.println("-------------------------");

  pinMode(Wifi_LED, OUTPUT);
  pinMode(Mqtt_LED, OUTPUT);
  digitalWrite(Wifi_LED, LOW);
  digitalWrite(Mqtt_LED, LOW);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("WiFi is connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);

  mqtt.subscribe(&LZ_Sts);
  mqtt.subscribe(&LZ_Tmr);

  pinMode(LightSwitch_Pin, INPUT_PULLUP);
  
}

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

bool IsMQTTConnected()
{
  static Timestamp last_ping;
  static Timestamp last_mqtt_attempt;
  static bool initial_mqtt = true;
  
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
      if (last_mqtt_attempt.Elapsed() > 60000 || initial_mqtt)
      {
        initial_mqtt = false;
        Serial.println("Trying to connect to MQTT Broker...");
        if (mqtt.connect() == 0) 
        {
          Serial.println("MQTT Connected.");
          digitalWrite(Mqtt_LED, HIGH);
          return true;
        }
        else
          Serial.println("MQTT connection failed");
        last_mqtt_attempt.Update();
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


void SetNeoPixelToTimeLeft(int time_left)
{
  double new_hue = ((Max_Hue_Time - time_left) / Max_Hue_Time) * Max_Hue;
  uint8_t rgb[3] = {0, 0, 0};
  if (time_left) converter.hslToRgb(new_hue, Saturation, Lightness, rgb);
  pixel.setPixelColor(0, pixel.Color(rgb[0], rgb[1], rgb[2]));
  pixel.show();
}

bool ButtonPressed()
{
  static bool last_button = false;
  static bool reported = false;
  static Timestamp press_time;
  
  bool button = digitalRead(LightSwitch_Pin);

  if (!reported)
  {
    if (!last_button && button)
    {
      press_time.Update();
      last_button = true;
    }
    else if (last_button && button)
    {
      if (press_time.Elapsed() >= Debounce_time)
      {
        reported = true;
        return true;   
      }
    }
  }

  if (!button) 
  {
    last_button = false;
    reported = false;
  }

  return false;
}

void loop() {

  static bool last_status = false;
  static int time_left = 0;
  
  
  if (IsMQTTConnected())
  {
    Adafruit_MQTT_Subscribe *subscription = 0;

    while (subscription = mqtt.readSubscription(10))
    {
      if (subscription == &LZ_Tmr)
      {
        time_left = atoi((const char*)subscription->lastread);

        if (last_status && time_left > 0)
        {
          SetNeoPixelToTimeLeft(time_left);
        }
      }

      if (subscription == &LZ_Sts)
      {
        //TODO make sure NeoPixel's state is consistant
        //e.g. if NeoPixel is off turn is on to some color

        if (strncasecmp((const char*)subscription->lastread, "OFF", 3) == 0)
        {
          SetNeoPixelToTimeLeft(0);
        }

        if (strncasecmp((const char*)subscription->lastread, "ON", 3) == 0)
        {
          last_status = true;
          SetNeoPixelToTimeLeft(time_left);
        }
      }
    }

    if (ButtonPressed())
    {
      LZ_Cmd.publish("ON");
    }
  }
}
