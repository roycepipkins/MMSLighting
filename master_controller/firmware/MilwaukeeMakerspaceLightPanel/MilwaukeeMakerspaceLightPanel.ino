#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <ETH.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <inttypes.h>
#include <MCP23S17.h>
#include <PubSubClient.h>
#include "MCP_IO.h"

#include "Timestamp.h"


#define DEMO_MAKERSPACE
#include "Credentials.h"

const int expansion_io_count = 0;


//TODO sign up for status request. Also publish status on boot

//***************************************************************************************
//EDIT values below for the correct panel number, etc 
//TODO you should probably take the number from an input on the board. A 4 bit DIP or something.

const char topic_prefix[] = "lighting/panel_1";
const char command_suffix[] = "/command";
const char status_suffix[] = "/status";
const char channel_prefix[] = "/channel_";
const char status_request_topic[] = "/lighting/status_request";

//Command topics will be topic_prefix + channel_prefix + channel_id + command_suffix
//E.x.: lighting/panel_1/channel_5/command
//Status topics will be topic_prefix + channel_prefix + channel_id + status_suffix
//E.x.: lighting/panel_1/channel_5/status

//***************************************************************************************




const int Wifi_LED = 33;
const int Mqtt_LED = 32;



// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char Mqtt_Server[]     = MQTT_SERVER;
const char Mqtt_Username[]   = MQTT_USERNAME;
const char Mqtt_Password[]   = MQTT_KEY;



SPIClass spi(HSPI);
const int slaveSelect = 14;
MCP_IO mcp_io(spi, slaveSelect);

const int connection_retries = 30;
int disconnect_counter = connection_retries;
void onMQTT(char* topic, byte* payload, unsigned int length);
WiFiClient client; //Still OK for Ethernet
PubSubClient pubSub(MQTT_SERVER, MQTT_SERVERPORT, onMQTT, client);




int strincmp(const char* s1, const char* s2, int len)
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

volatile bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      {
        String hostname = "MstrLght_";
        hostname += ETH.macAddress();
        ETH.setHostname(hostname.c_str());
      }
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}


void setup() {

  Serial.begin(115200);
  delay(10);

  Serial.println("");
  Serial.println("");
  Serial.println("----------------------");
  Serial.println("MMS Master Light Panel");
  Serial.println("         v3.0         ");
  Serial.println("----------------------");


  pinMode(Wifi_LED, OUTPUT);
  pinMode(Mqtt_LED, OUTPUT);
  digitalWrite(Wifi_LED, LOW);
  digitalWrite(Mqtt_LED, LOW);

  spi.begin(13, 4, 3);
  spi.setFrequency(10000);
  for(int i = 0; i < (16 + 16*expansion_io_count); ++i)
  {
    mcp_io.pinMode(i, OUTPUT);
  }

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Ethernet starting.");
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  
  //With the High Voltage bypass switches, there are no local buttons.
  //With no local buttons, an active network is the only useful state.
  int retries = 30;
  while(!eth_connected)
  {
    delay(1000);
    Serial.print(".");
    if (retries == 0)
    {
      Serial.println("");
      Serial.println("Ethernet didn't start in time. Restarting in five seconds.");
      delay(5000);
      ESP.restart();
    }
  }


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
  
  ArduinoOTA.setHostname(ETH.getHostname());
#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword(OTA_PASSWORD);
#endif

  Serial.println("Connecting to MQTT...");
  if (pubSub.connect(ETH.getHostname()))
  {
    Serial.println("Connected to MQTT.");
    subscribeToTopics();
  }
  else
  {
    Serial.println("Failed to connected to MQTT!");
  }

  Serial.println("Starting Light Controller.");
}

void subscribeToTopics()
{
  String subscription_topic(topic_prefix);
  subscription_topic += "/+";
  subscription_topic += command_suffix;

  if (pubSub.subscribe(subscription_topic.c_str(), 1))
  {
    Serial.print("Subscribed to: ");
    Serial.println(subscription_topic);
  }
  else
  {
    Serial.println("Failed to subscribe! Will restart in five seconds...");
    delay(5000);
    ESP.restart();
  }
  
  if (pubSub.subscribe(status_request_topic, 1))
  {
    Serial.print("Subscribed to: ");
    Serial.println(subscription_topic);
  }
  else
  {
    Serial.println("Failed to subscribe! Will restart in five seconds...");
    delay(5000);
    ESP.restart();
  }

}

int payloadToCommand(byte* payload)
{
  //Infer the meaning of the payload
  if (payload[0] == 0) return 0;
  if (payload[0] == 1) return 1;
  if (payload[0] == '0') return 0;
  if (payload[0] == '1') return 1;
  char* data = (char*)payload;
  if (strincmp(data, "on", 2) == 0) return 1;
  if (strincmp(data, "true", 4) == 0) return 1;
  if (strincmp(data, "high", 4) == 0) return 1;
  return 0;
}

void publishAllStatus()
{
  for(int channel = 0; channel < 16; ++channel)
  {
    String status = mcp_io.digitalRead(channel) != 0 ? "ON" : "OFF";
    String status_topic(topic_prefix);
    status_topic += channel_prefix;
    status_topic += channel;
    status_topic += status_suffix;
    pubSub.publish(status_topic.c_str(), status.c_str());
  }
}


void onMQTT(char* topic, byte* payload, unsigned int length)
{
  if (strcmp(status_request_topic, topic) == 0)
  {
    publishAllStatus();
  }
  else
  {
    String subscription_prefix(topic_prefix);
    subscription_prefix += channel_prefix;
    int prefix_len = subscription_prefix.length();
  
    if (strlen(topic) > prefix_len && 
        isdigit(topic[prefix_len] && 
        length > 0) && 
        (0 == strincmp(topic, subscription_prefix.c_str(), prefix_len)))
    {
      int channel = atoi(&topic[prefix_len]);
      int command = payloadToCommand(payload); 
      mcp_io.digitalWrite(channel, command);
      String status = mcp_io.digitalRead(channel) != 0 ? "ON" : "OFF";
      String status_topic(topic_prefix);
      status_topic += channel_prefix;
      status_topic += channel;
      status_topic += status_suffix;
      pubSub.publish(status_topic.c_str(), status.c_str());
      
      
      Serial.print("Commanded Channel ");
      Serial.print(channel);
      Serial.print(" to ");
      Serial.println(command);
    }
  } 
}

void reconnectPubSub()
{
  Serial.println("Reconnecting to MQTT Server...");
  if (pubSub.connect(ETH.getHostname()))
  {
    Serial.println("Connected to MQTT Server");
    subscribeToTopics();
  }
  else
  {
    Serial.println("Failed to connect MQTT Server");
  }
  
}


void loop()
{
  if (eth_connected)
  {
    disconnect_counter = connection_retries;
    ArduinoOTA.handle();
    if (!pubSub.loop())
    {
      reconnectPubSub();
    }
  }
  else
  {
    if (--disconnect_counter == 0)
    {
      Serial.println("Ethernet still not connected. Restarting in five seconds");
      delay(5000);
      ESP.restart();
    }
    else
    {
      Serial.println("Ethernet not connected. Waiting for better times.");
      delay(1000);
    }
  }
}
