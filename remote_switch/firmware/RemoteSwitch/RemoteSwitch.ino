#include <inttypes.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Timestamp.h"
#include <math.h>
#include <Adafruit_NeoPixel.h>

#define AT_MAKERSPACE
#include "Credentials.h"

#define LIGHT_ZONE 7
const char ota_hostname[]  = "LZ7SW1";

const char Mqtt_Server[]     = MQTT_SERVER;
const char Mqtt_Username[]  = MQTT_USERNAME;
const char Mqtt_Password[]   = MQTT_KEY;



#if LIGHT_ZONE == 1
const char LZ_Timer_Topic[]  = "Lighting/LZ1_Timer";
const char LZ_Sts_Topic[]  = "Lighting/LZ1_Sts";
const char LZ_Cmd_Topic[]  = "Lighting/LZ1_Cmd";
#elif LIGHT_ZONE == 2
const char LZ_Timer_Topic[]  = "Lighting/LZ2_Timer";
const char LZ_Sts_Topic[]  = "Lighting/LZ2_Sts";
const char LZ_Cmd_Topic[]  = "Lighting/LZ2_Cmd";
#elif LIGHT_ZONE == 3
const char LZ_Timer_Topic[]  = "Lighting/LZ3_Timer";
const char LZ_Sts_Topic[]  = "Lighting/LZ3_Sts";
const char LZ_Cmd_Topic[]  = "Lighting/LZ3_Cmd";
#elif LIGHT_ZONE == 4
const char LZ_Timer_Topic[]  = "Lighting/LZ4_Timer";
const char LZ_Sts_Topic[]  = "Lighting/LZ4_Sts";
const char LZ_Cmd_Topic[]  = "Lighting/LZ4_Cmd";
#elif LIGHT_ZONE == 5
const char LZ_Timer_Topic[]  = "Lighting/LZ5_Timer";
const char LZ_Sts_Topic[]  = "Lighting/LZ5_Sts";
const char LZ_Cmd_Topic[]  = "Lighting/LZ5_Cmd";
#elif LIGHT_ZONE == 6
const char LZ_Timer_Topic[]  = "Lighting/LZ6_Timer";
const char LZ_Sts_Topic[]  = "Lighting/LZ6_Sts";
const char LZ_Cmd_Topic[]  = "Lighting/LZ6_Cmd";
#elif LIGHT_ZONE == 7
const char LZ_Timer_Topic[]  = "Lighting/LZ7_Timer";
const char LZ_Sts_Topic[]  = "Lighting/LZ7_Sts";
const char LZ_Cmd_Topic[]  = "Lighting/LZ7_Cmd";
#elif LIGHT_ZONE == 8
const char LZ_Timer_Topic[]  = "Lighting/LZ8_Timer";
const char LZ_Sts_Topic[]  = "Lighting/LZ8_Sts";
const char LZ_Cmd_Topic[]  = "Lighting/LZ8_Cmd";
#endif



const int LightSwitch_Pin = 12;
const int Wifi_LED = 16;
const int Mqtt_LED = 14; 
const int NeoPixel_Pin = 13;

const int Debounce_time = 75;
const int Blink_time = 250;
const int Long_Press_Time = 5000;
const bool Long_Press_Enabled = true;

const int Max_Hue_Time = 180; //amount of time remaining for light to be green

//TODO set to a nice green
const double Max_Hue = 115; 
const double Saturation = 1.0;
const double Lightness = 0.5;

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, Mqtt_Server, MQTT_SERVERPORT, Mqtt_Username, Mqtt_Password);
Adafruit_MQTT_Publish LZ_Cmd = Adafruit_MQTT_Publish(&mqtt, LZ_Cmd_Topic, 1);
Adafruit_MQTT_Subscribe LZ_Sts = Adafruit_MQTT_Subscribe(&mqtt, LZ_Sts_Topic, 1);
Adafruit_MQTT_Subscribe LZ_Tmr = Adafruit_MQTT_Subscribe(&mqtt, LZ_Timer_Topic, 1);

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, NeoPixel_Pin, NEO_RGB + NEO_KHZ800);






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

  pixel.begin();
  pixel.show();

  SetNeoPixelToTimeLeft(0);

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");
  });
  
  ArduinoOTA.setHostname(ota_hostname);
#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword(OTA_PASSWORD);
#endif 
}

double floatmod(double a, double b)
{
    return (a - b * floor(a / b));
}

void HSL2RGB(double h, double s, double l, uint8_t rgb[])
{
  double c = (1 - fabs(2 * l - 1)) * s;
  h /= 60;
  double x = c*(1 - fabs(floatmod(h, 2) - 1));

  double r1, g1, b1;
  if (h >= 0 && h < 1)
  {
    r1 = c;
    g1 = x;
    b1 = 0;
  }

  if (h >= 1 && h < 2)
  {
    r1 = x;
    g1 = c;
    b1 = 0;
  }

  if (h >= 2 && h < 3)
  {
    r1 = 0;
    g1 = c;
    b1 = x;
  }

  if (h >= 3 && h < 4)
  {
    r1 = 0;
    g1 = x;
    b1 = c;
  }

  if (h >= 4 && h < 5)
  {
    r1 = x;
    g1 = 0;
    b1 = c;
  }

  if (h >= 5 && h < 6)
  {
    r1 = c;
    g1 = 0;
    b1 = x;
  }

  double m = l - 0.5*c;

  rgb[0] = 255*(r1 + m);
  rgb[1] = 255*(g1 + m);
  rgb[2] = 255*(b1 + m);

}

int mystrncasecmp(const char* s1, const char* s2, int len)
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
  static bool initial_wifi = true;
  bool ret = false;
  
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
      ret = true;
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
          ret = true;
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

  return ret;
}


void SetNeoPixelToTimeLeft(int time_left)
{
  if (time_left > Max_Hue_Time) time_left = Max_Hue_Time;
  double new_hue = ((double)time_left / (double)Max_Hue_Time) * Max_Hue;
  Serial.print("time_left: ");
  Serial.print(time_left);
  Serial.print(" new_hue: ");
  Serial.println(new_hue);
  uint8_t rgb[3] = {0, 0, 0};
  if (time_left) HSL2RGB(new_hue, Saturation, Lightness, rgb);
  Serial.print("setPixelColor: r: ");
  Serial.print(rgb[0]);
  Serial.print(" g: ");
  Serial.print(rgb[1]);
  Serial.print(" b: ");
  Serial.println(rgb[2]);
  pixel.setPixelColor(0, pixel.Color(rgb[0], rgb[1], rgb[2]));
  
  pixel.show();
}

bool ButtonPressed(uint32_t& time_pressed)
{
  static bool last_button = false;
  static bool reported = false;
  static Timestamp press_time;
  
  bool button = !digitalRead(LightSwitch_Pin);

  if (!reported)
  {
    if (!last_button && button)
    {
      press_time.Update();
      last_button = true;
    }
    else if (last_button && !button)
    {
      if (press_time.Elapsed() >= Debounce_time)
      {
        time_pressed = press_time.Elapsed();
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

void RunPendingOff()
{
  static Timestamp blink_time;
  static bool last_state = false;

  if (blink_time.Elapsed() >= Blink_time)
  {
    last_state = !last_state;
    blink_time.Update();

    SetNeoPixelToTimeLeft(last_state ? 5 : 0);
  }
}

void loop() {

  static bool last_status = false;
  static int time_left = 0;
  static bool pending_off = false;
  static bool initial_wifi = true;

  if (WiFi.status() == WL_CONNECTED)
  {
    if (initial_wifi)
    {
      ArduinoOTA.begin();
      Serial.println("OTA Begin");
      Serial.print("My IP address: ");
      Serial.println(WiFi.localIP());
      initial_wifi = false;
    }
    else
    {
      ArduinoOTA.handle();
    }
  }

  
  
  if (IsMQTTConnected())
  {
    Adafruit_MQTT_Subscribe *subscription = 0;

    while (subscription = mqtt.readSubscription(10))
    {
      if (subscription == &LZ_Tmr)
      {
        time_left = atoi((const char*)subscription->lastread);

         Serial.print("time_left: ");
         Serial.println(time_left);
        
        if (time_left > 0)
        {
          SetNeoPixelToTimeLeft(time_left);
        }
      }

      if (subscription == &LZ_Sts)
      {
        //TODO make sure NeoPixel's state is consistant
        //e.g. if NeoPixel is off turn is on to some color

        if (mystrncasecmp((const char*)subscription->lastread, "OFF", 3) == 0)
        {
          pending_off = false;
          SetNeoPixelToTimeLeft(0);
        }

        if (mystrncasecmp((const char*)subscription->lastread, "ON", 2) == 0)
        {
          last_status = true;
          pending_off = false;
          SetNeoPixelToTimeLeft(time_left);
        }

        if (mystrncasecmp((const char*)subscription->lastread, "Pending_Off", 11) == 0)
        {
          pending_off = true;
        }
      }
    }

    uint32_t time_pressed = 0;
    if (ButtonPressed(time_pressed))
    {
      if (time_pressed >= Long_Press_Time && Long_Press_Enabled)
        LZ_Cmd.publish("Off");
      else
        LZ_Cmd.publish("On");
    }
  }

  if (pending_off) RunPendingOff();

  
}
