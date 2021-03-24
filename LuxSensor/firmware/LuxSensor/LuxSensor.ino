#include "Adafruit_VEML7700.h"
#include "ESP8266WiFi.h"
#include <PubSubClient.h>
#include "Credentials.h"
//#include "WiFiManager.h"
#include <EEPROM.h>
#include "Timestamp.h"

//WiFiManager wifiManager;

Adafruit_VEML7700 veml = Adafruit_VEML7700();
const int Wifi_LED = 16;
const int Mqtt_LED = 14;

const int connection_retries = 30;
int disconnect_counter = connection_retries;
void onMQTT(char* topic, byte* payload, unsigned int length);
WiFiClient client; //Still OK for Ethernet
PubSubClient pubSub(MQTT_SERVER, MQTT_SERVERPORT, onMQTT, client);

char host[40] = "ds1";
char mqtt_server[40];
char mqtt_port[6]  = "1883";
bool configSaveNeeded = false;
char topic_prefix[] = "lighting";
String gain_topic;
String integration_topic;
String report_topic;
int report_period_secs = 300;
Timestamp report_timer;


void publishSensor()
{
  String lux_topic(topic_prefix);
  lux_topic += "/";
  lux_topic += host;
  lux_topic += "/lux";
  String lux;
  lux += veml.readLux();
  pubSub.publish(lux_topic.c_str(), lux.c_str());
}

void subscribeOrDie(const char* topic, int qos)
{
  if (pubSub.subscribe(topic, qos))
  {
    Serial.print("Subscribed to: ");
    Serial.println(topic);
  }
  else
  {
    Serial.println("Failed to subscribe! Will restart in five seconds...");
    delay(5000);
    ESP.restart();
  }
}

void subscribeToTopics()
{
  String subscription_topic(topic_prefix);
  subscription_topic += "/";
  subscription_topic += host;
  subscription_topic += "/";

  gain_topic = subscription_topic;
  gain_topic += "gain";
  subscribeOrDie(gain_topic.c_str(), 1);
  
  integration_topic = subscription_topic;
  integration_topic += "integration";
  subscribeOrDie(integration_topic.c_str(), 1);

  report_topic = subscription_topic;
  report_topic += "report_period";
  subscribeOrDie(report_topic.c_str(), 1);
  
}


void onMQTT(char* topic, byte* payload, unsigned int length)
{
  Serial.print("MQTT Topic: ");
  Serial.println(topic);
  
  if (strcmp(gain_topic.c_str(), topic) == 0)
  {
    int new_gain_enum = atoi((const char*)payload);
    veml.setGain(new_gain_enum);
  }
  else if (strcmp(integration_topic.c_str(), topic) == 0)
  {
    int new_inttime_enum = atoi((const char*)payload);
    veml.setIntegrationTime(new_inttime_enum);
  }
  else if (strcmp(report_topic.c_str(), topic) == 0)
  {
    report_period_secs = max(atoi((const char*)payload), 1);
  }
}

void reconnectPubSub()
{
  Serial.println("Reconnecting to MQTT Server...");
  if (pubSub.connect(host))
  {
    digitalWrite(Mqtt_LED, HIGH);
    Serial.println("Connected to MQTT Server");
    subscribeToTopics();
  }
  else
  {
    digitalWrite(Mqtt_LED, LOW);
    Serial.println("Failed to connect MQTT Server");
  }
  
}

void saveConfigCallback () {
  Serial.println("Should save config");
  configSaveNeeded = true;
}

/*void runWifiManager()
{
  uint32_t first_run = 0;
  int offset = 0;
  EEPROM.begin(256);
  EEPROM.get(offset, first_run);
  offset += sizeof(first_run);

  if (first_run == 0xdeadbeef)
  {
    EEPROM.get(offset, mqtt_server);
    offset += sizeof(mqtt_server);
    EEPROM.get(offset, mqtt_port);
    offset += sizeof(mqtt_port);
    EEPROM.get(offset, host);
    offset += sizeof(host);  
  }
  
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, sizeof(mqtt_server));
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, sizeof(mqtt_port));
  WiFiManagerParameter custom_host("host", "host name", host, sizeof(host));

  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_host);
  wifiManager.setSaveParamsCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(600);
  wifiManager.setConnectTimeout(10);
  
  if (!wifiManager.autoConnect())
  {
    Serial.println("Failed to connect in time. (10 mins)");
    delay(10000);
    ESP.restart();
  }

  if (configSaveNeeded)
  {
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(host, custom_host.getValue());
    Serial.println("Saving:");
    Serial.print("mqtt_server: ");
    Serial.println(mqtt_server);
    Serial.print("mqtt_port: ");
    Serial.println(mqtt_port);
    Serial.print("host: ");
    Serial.println(host);
    
    offset = 0;
    first_run = 0xdeadbeef;
    EEPROM.put(offset, first_run);
    offset += sizeof(first_run);
    EEPROM.put(offset, mqtt_server);
    offset += sizeof(mqtt_server);
    EEPROM.put(offset, mqtt_port);
    offset += sizeof(mqtt_port);
    EEPROM.put(offset, host);
    offset += sizeof(host);
    EEPROM.commit();
  }

  EEPROM.end();
}*/

void setup() {

  pinMode(Wifi_LED, OUTPUT);
  pinMode(Mqtt_LED, OUTPUT);
  digitalWrite(Wifi_LED, LOW);
  digitalWrite(Mqtt_LED, LOW);

  Serial.begin(115200);
  Serial.println(); Serial.println();
  Serial.println("Lux Sensor v1.0");

  
  //runWifiManager();
  //digitalWrite(Wifi_LED, HIGH);

  
  Wire.begin(12, 13);
  

  

  if (!veml.begin(&Wire, false)) {
    Serial.println("Sensor not found");
    while (1);
  }
  Serial.println("Sensor found");

  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_800MS);

  Serial.print(F("Gain: "));
  switch (veml.getGain()) {
    case VEML7700_GAIN_1: Serial.println("1"); break;
    case VEML7700_GAIN_2: Serial.println("2"); break;
    case VEML7700_GAIN_1_4: Serial.println("1/4"); break;
    case VEML7700_GAIN_1_8: Serial.println("1/8"); break;
  }

  Serial.print(F("Integration Time (ms): "));
  switch (veml.getIntegrationTime()) {
    case VEML7700_IT_25MS: Serial.println("25"); break;
    case VEML7700_IT_50MS: Serial.println("50"); break;
    case VEML7700_IT_100MS: Serial.println("100"); break;
    case VEML7700_IT_200MS: Serial.println("200"); break;
    case VEML7700_IT_400MS: Serial.println("400"); break;
    case VEML7700_IT_800MS: Serial.println("800"); break;
  }

  Serial.println(); Serial.println();
  Serial.print("SSID: ");
  Serial.println(WLAN_SSID);
  Serial.print("WiFi starting.");
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  
  Timestamp initialWifiConnect;
  while(!WiFi.isConnected())
  {
    delay(1000);
    Serial.print(".");
    if (initialWifiConnect.Elapsed() > 10000)
    {
      Serial.println("Too long to connect. Restarting");
      delay(2000);
      ESP.restart();
    }
  }
  Serial.println("\nConnected.");
  Serial.println(WiFi.localIP().toString());

  Serial.println("Connecting to MQTT...");
  if (pubSub.connect(host))
  {
    digitalWrite(Mqtt_LED, HIGH);
    Serial.println("Connected to MQTT.");
    subscribeToTopics();
  }
  else
  {
    digitalWrite(Mqtt_LED, LOW);
    Serial.println("Failed to connected to MQTT!");
  }
}

void loop() {
  Serial.print("Lux: "); Serial.println(veml.readLux());
  Serial.print("White: "); Serial.println(veml.readWhite());
  Serial.print("Raw ALS: "); Serial.println(veml.readALS());


  if (WiFi.isConnected())
  {
    digitalWrite(Wifi_LED, HIGH);
    disconnect_counter = connection_retries;
    
    if (!pubSub.loop())
    {
      reconnectPubSub();
    }
    else
    {
      if (report_timer.Elapsed() > report_period_secs*1000)
      {
        publishSensor();
        report_timer.Update();
      }
    }
  }
  else
  {
    digitalWrite(Wifi_LED, LOW);
    if (--disconnect_counter == 0)
    {
      Serial.println("Network still not connected. Restarting in five seconds");
      delay(5000);
      ESP.restart();
    }
    else
    {
      Serial.println("Network not connected. Waiting for better times.");
      delay(1000);
    }
  }

  delay(1000);
}
