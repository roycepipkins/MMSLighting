#include <inttypes.h>
#include <vector>
#include <WiFi.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Timestamp.h"
#include <math.h>
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h>  
#include "Image565.h"
#include "ON.h"
#include "OFF.h"
#include "MQTT_Icon.h"
#include "No_MQTT_Icon.h"
#include "Wifi_Icon.h"
#include "No_Wifi_Icon.h"
#include "Pending_OFF_Green.h"
#include "Pending_OFF_Red.h"
#include "ScopedLock.h"
#include "DBButton.h"
#include <time.h>
#include "NTPTime.h"


SPIClass hspi(HSPI);

//Adafruit_ST7735 tft = Adafruit_ST7735(&hspi, 16, 17, 9 ); // CS,A0,SDA,SCK,RESET
Adafruit_ST7735 tft = Adafruit_ST7735(16, 17, 23, 5, 9); // CS,A0,SDA,SCK,RESET

#define AT_HOME
#include "Credentials.h"


const char Mqtt_Server[]    = MQTT_SERVER;
const char Mqtt_Username[]  = MQTT_USERNAME;
const char Mqtt_Password[]  = MQTT_KEY;

const char ntp_server[] = "10.1.1.1";
const char timezone_desc[] = "CST6CDT,M3.2.0,M11.1.0";

NTPTime ntp_time;

const std::vector<std::string> LZ_Sts_Topics 
{
  "Lighting/LZ1_Sts",
  "Lighting/LZ2_Sts",
  "Lighting/LZ3_Sts",
  "Lighting/LZ4_Sts",
  "Lighting/LZ5_Sts",
  "Lighting/LZ6_Sts",
  "Lighting/LZ7_Sts",
  "Lighting/LZ8_Sts"
};

const std::vector<std::string> LZ_Cmd_Topics  = 
{
  "Lighting/LZ1_Cmd",
  "Lighting/LZ2_Cmd",
  "Lighting/LZ3_Cmd",
  "Lighting/LZ4_Cmd",
  "Lighting/LZ5_Cmd",
  "Lighting/LZ6_Cmd",
  "Lighting/LZ7_Cmd",
  "Lighting/LZ8_Cmd"
};


const int Button_A_Pin = 34;
const int Button_B_Pin = 35;
const int Button_C_Pin = 39;

const int Debounce_time = 75;

DBButton Button_ON(Button_C_Pin, Debounce_time, true, true);
DBButton Button_OFF(Button_B_Pin, Debounce_time, true, true);
DBButton Button_PRGM(Button_A_Pin, Debounce_time, true, true);



WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, Mqtt_Server, MQTT_SERVERPORT, Mqtt_Username, Mqtt_Password);


Adafruit_MQTT_Publish MQTTPing = Adafruit_MQTT_Publish(&mqtt, "Lighting/Ping", 0);

std::vector<Adafruit_MQTT_Publish> LZ_Cmd;
std::vector<Adafruit_MQTT_Subscribe> LZ_Sts;

uint8_t zone_index = 5;

SemaphoreHandle_t mu_tft;
EventGroupHandle_t status_bits_handle;
const int ZONE_STATUS_BITS        = 31; 
const int ZONE_STATUS_OFF         = 1;
const int ZONE_STATUS_ON          = 2;
const int ZONE_STATUS_PENDING_OFF = 4;
const int ZONE_PROGRAMMING        = 8;
const int TIME_TICK               = 16;

const int status_x_offset = 15;
const int status_y_offset = 20;
const int prgm_x_offset = 60;
const int prgm_y_offset = 0;

#define SPEAKER_PIN 25
#define TONE_PIN_CHANNEL 0
#define BEEP_FREQ 1000

void SpeakerSetup()
{
  ledcSetup(TONE_PIN_CHANNEL, 0, 13);
  ledcAttachPin(SPEAKER_PIN, TONE_PIN_CHANNEL);
}

void Beep()
{
  ledcWriteTone(TONE_PIN_CHANNEL, BEEP_FREQ);
}

void Mute()
{
  ledcWriteTone(TONE_PIN_CHANNEL, 0);
  digitalWrite(SPEAKER_PIN, 0);
}

void UpdateStatus(void*)
{
  SpeakerSetup();

  while(true)
  {
    EventBits_t status_bits = xEventGroupWaitBits(status_bits_handle, ZONE_STATUS_BITS, pdTRUE, pdFALSE, portMAX_DELAY);
    if (status_bits & ZONE_PROGRAMMING)
    {
      Beep();
      ScopedLock locker(mu_tft);
      tft.fillRect(prgm_x_offset, prgm_y_offset, 16, 16, 0);
      tft.drawChar(prgm_x_offset, prgm_y_offset, zone_index + 49, ST7735_GREEN,ST7735_BLACK, 2);
      Mute();
    }
    else if (status_bits & ZONE_STATUS_ON)
    {
      Beep();
      ScopedLock locker(mu_tft);
      on_image.draw(tft, status_x_offset, status_y_offset);
      Mute();
    }
    else if (status_bits & ZONE_STATUS_OFF)
    {
      Beep();
      ScopedLock locker(mu_tft);
      off_image.draw(tft, status_x_offset, status_y_offset);
      Mute();
    }
    else if (status_bits & ZONE_STATUS_PENDING_OFF)
    {
      {
        ScopedLock locker(mu_tft);
        Pending_OFF_Green.draw(tft, status_x_offset, status_y_offset);
      }
      while(!((ZONE_STATUS_OFF | ZONE_STATUS_ON) & xEventGroupGetBits(status_bits_handle)))
      {
        {
          ScopedLock locker(mu_tft);
          tft.invertDisplay(true);
        }
        Beep();
        vTaskDelay(200);
        Mute();
        {
          ScopedLock locker(mu_tft);
          tft.invertDisplay(false);
        }
        
        vTaskDelay(200);
        
      }
    }
    else if (status_bits & TIME_TICK)
    {
      drawTime();
    }
  }
}

void drawTime()
{
  int x_offset = 50;
  const int y_offset = 110;
  auto time_str = ntp_time.current_local_hhmmss();

  ScopedLock locker(mu_tft);
  for(int i=0; i < time_str.size(); ++i, x_offset += 10)
  {
    tft.drawChar(x_offset, y_offset, time_str[i], ST7735_WHITE,ST7735_BLACK, 2);
  }
  
}

void setup() {
  status_bits_handle = xEventGroupCreate();
  mu_tft = xSemaphoreCreateMutex();
  
  hspi.setFrequency(80000000);
  hspi.begin(5, -1, 23, -1);

  Serial.begin(115200);
  delay(10);

  Serial.println("");
  Serial.println("");
  Serial.println("-------------------------");
  Serial.println("MMS Remote Light Switch  ");
  Serial.println("         v2.1            ");
  Serial.println("-------------------------");


  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("WiFi is connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  WiFi.setAutoReconnect(true);

  Serial.println("WiFi begin has been called ");

  //Create Status and Command topic objects
  for(int i = 0; i < 8; ++i)
  {
    LZ_Sts.emplace_back(&mqtt, LZ_Sts_Topics[i].c_str(), 1);
    LZ_Cmd.emplace_back(&mqtt, LZ_Cmd_Topics[i].c_str(), 1);
  }
  
  Serial.println("MQTT emplacements made");

  //Get the Status objects subscribed
  for(auto& zone_status : LZ_Sts)
  {
    mqtt.subscribe(&zone_status);
  }
  
  Serial.println("MQTT subscriptions made");


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
  
  ArduinoOTA.setHostname(String("ESP" + String((unsigned long)ESP.getEfuseMac(), HEX)).c_str());

  Serial.println("ArduinoOTA has been setup");

  xTaskCreatePinnedToCore(UpdateStatus, "UpdateStatusTask", 8192, nullptr, 1, nullptr, 0);

  SPIFFS.begin(true);
  if (SPIFFS.exists("/zone_index"))
  {
    File zif = SPIFFS.open("/zone_index");
    if (zif.available())
    {
      zif.read((uint8_t*)&zone_index, 1);
    }
    zif.close();
  }

  tft.initR(INITR_18GREENTAB);                             // 1.44 v2.1
  tft.fillScreen(ST7735_BLACK);                            // CLEAR
  tft.setTextColor(0x5FCC);                                // GREEN
  tft.setRotation(1);                                      // 
  
  tft.drawChar(prgm_x_offset, prgm_y_offset, zone_index + 49, ST7735_GREEN,ST7735_BLACK, 2);


  ntp_time.begin(ntp_server, timezone_desc);

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

const int wifi_x_offset = 112;
const int wifi_y_offset = 0;

void DisplayWifiStatus(bool status)
{
  ScopedLock locker(mu_tft);
  if (status)
  {
    wifi_icon.draw(tft, wifi_x_offset, wifi_y_offset);
  }
  else
  {
    no_wifi_icon.draw(tft, wifi_x_offset, wifi_y_offset);
  }
}

const int mqtt_x_offset = 2;
const int mqtt_y_offset = 0;


void DisplayMQTTStatus(bool status)
{
  ScopedLock locker(mu_tft);
  if (status)
  {
    mqtt_icon.draw(tft, mqtt_x_offset, mqtt_y_offset);
  }
  else
  {
    no_mqtt_icon.draw(tft, mqtt_x_offset, mqtt_y_offset);
  }
}



bool IsMQTTConnected()
{
  static Timestamp ten_secs;
  static Timestamp wifi_down;
  static Timestamp last_ping;
  static Timestamp last_mqtt_attempt;
  static Timestamp mqtt_down;
  static bool initial_mqtt = true;
  static bool initial_wifi = true;
  bool ret = false;
  
  if (ten_secs.Elapsed() >= 10000)
  {
    ten_secs.Update();
    //Serial.println("10 sec timer");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    DisplayWifiStatus( HIGH);
    wifi_down.Update();

    if (mqtt.connected()) 
    {
      DisplayMQTTStatus(HIGH);
      mqtt_down.Update();
      if (last_ping.Elapsed() >= 30000)
      {
        last_ping.Update();
        MQTTPing.publish(String("ESP" + String((unsigned long)ESP.getEfuseMac(), HEX)).c_str());
      }
      ret = true;
    }
    else
    {
      DisplayMQTTStatus(LOW);
      mqtt.disconnect();
      if (last_mqtt_attempt.Elapsed() > 60000 || initial_mqtt)
      {
        initial_mqtt = false;
        Serial.println("Trying to connect to MQTT Broker...");
        if (mqtt.connect() == 0) 
        {
          Serial.println("MQTT Connected.");
          
          DisplayMQTTStatus(HIGH);
          ret = true;
        }
        else
        {
          Serial.println("MQTT connection failed");
          DisplayMQTTStatus(LOW);
        }
          
        last_mqtt_attempt.Update();
      }
    }
  }
  else
  {
    DisplayWifiStatus(LOW);
    DisplayMQTTStatus(LOW);
    mqtt.disconnect();

    if (wifi_down.Elapsed() > 30000 ||
        mqtt_down.Elapsed() > 30000)
    {
      ESP.restart();
    }
  }

  return ret;
}


void loop() {

  static bool last_status = false;
  static int time_left = 0;
  static bool pending_off = false;
  static bool initial_wifi = true;
  static Timestamp loop_timer;

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
    last_status = true;
  }
  else
  {
    if (last_status)
    {
      loop_timer.Update();
    }
    else
    {
      if (loop_timer.Elapsed() > 30000)
      {
        ESP.restart();
      }
    }
    last_status = false;
  }
  
  
  if (IsMQTTConnected())
  {
    Adafruit_MQTT_Subscribe *subscription = 0;

    while (subscription = mqtt.readSubscription(10))
    {
      if (subscription == &(LZ_Sts[zone_index]))
      {
        //TODO make sure NeoPixel's state is consistant
        //e.g. if NeoPixel is off turn is on to some color

        if (mystrncasecmp((const char*)subscription->lastread, "OFF", 3) == 0)
        {
          xEventGroupSetBits(status_bits_handle, ZONE_STATUS_OFF);
        }

        if (mystrncasecmp((const char*)subscription->lastread, "ON", 2) == 0)
        {
          xEventGroupSetBits(status_bits_handle, ZONE_STATUS_ON);
        }

        if (mystrncasecmp((const char*)subscription->lastread, "Pending_Off", 11) == 0)
        {
          xEventGroupSetBits(status_bits_handle, ZONE_STATUS_PENDING_OFF);
        }
      }
    }

    

  
    
    
    if (Button_ON.isPressed())
    {
        LZ_Cmd[zone_index].publish("On");
        Serial.print("Zone #");
        Serial.print(zone_index + 1);
        Serial.println(" -> On");
        
    }

  
    if (Button_OFF.isPressed())
    {
        LZ_Cmd[zone_index].publish("Off");
        Serial.print("Zone #");
        Serial.print(zone_index + 1);
        Serial.println(" -> Off");
        
    }

    if (Button_PRGM.isPressed())
    {
      if (++zone_index > 7) zone_index = 0;
      xEventGroupSetBits(status_bits_handle, ZONE_PROGRAMMING);
      File zif = SPIFFS.open("/zone_index", FILE_WRITE);
      zif.write((uint8_t*)&zone_index, 1);
      zif.close();

      Serial.print("Zone #");
      Serial.print(zone_index + 1);
      Serial.println(" -> Programmed");
      
    }
  }

  static Timestamp print_timer;
  if (print_timer.Elapsed() > 1000)
  {
    Serial.println(ntp_time.current_local_hhmmss().c_str());
    print_timer.Update();
    xEventGroupSetBits(status_bits_handle, TIME_TICK);
  }

  tm now = {0};
  ntp_time.current_local_time(now);
  if (now.tm_hour == 2 && 
      now.tm_min == 0 && 
      now.tm_sec < 5 && 
      loop_timer.Elapsed() > 60000)
  {
    ESP.restart();
  }

}
