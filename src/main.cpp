#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Secrets.h"

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else 
static const BaseType_t app_cpu = 1;
#endif

static const std::uint8_t queue_length = 10;

static QueueHandle_t data_queue;

const char * ssid     = Secrets::ssid;
const char * password = Secrets::pass;

const char * mqtt_host = Secrets::mqtt_host;
const int mqtt_port    = Secrets::mqtt_port;

String client_id = "esp_client";

WiFiClient client;
PubSubClient mqtt_client(client); 

#define RX2 16
#define TX2 17

void callback(char *topic, byte *payload, unsigned int lenght);
void ConnectToWiFi();
void reconnect();

StaticJsonDocument<96> doc;
std::string jsonOut;

#pragma pack(1)
typedef struct Payloads {
    std::uint8_t moist;
    float env_temp;
    float env_hum;
    std::uint16_t soil_temp;
    std::uint8_t light;
}Payload;
#pragma pack()
TaskHandle_t task1;

char buff[sizeof(Payload)];
Payload * _pl;


void ReadSerial(void * parameter){
  while (1)
  {    
    if(Serial.available() > 0){
          
      Serial.readBytes(buff, sizeof(Payload));
      if(xQueueSend(data_queue, &buff, 10) != pdTRUE){
        Serial.println("Error message!");
      }
    }
  
  }
}

void setup() {
  data_queue = xQueueCreate(queue_length, sizeof(buff));
  Serial.begin(9600);
  Serial.println("hello");


  xTaskCreatePinnedToCore(
    ReadSerial,
    "Serial_task",
    1024,
    NULL,
    1,
    NULL,
    app_cpu);

  ConnectToWiFi();
  mqtt_client.setServer(mqtt_host, mqtt_port);
  mqtt_client.setCallback(callback);

}


void loop() {
  char local_buff[sizeof(buff)];
  if(!mqtt_client.connected()){
    reconnect();
  }

  if(xQueueReceive(data_queue, &local_buff, 0) == pdTRUE){ 
      _pl = (Payload *)(local_buff);
      doc["id"] = 1;
      doc["moist"] = _pl->moist;
      doc["light"] = _pl->light;
      doc["envTemp"] = _pl->env_temp;
      doc["envHum"] = _pl->env_hum;
      doc["soilTemp"] = _pl->soil_temp;
      jsonOut = "";
      serializeJson(doc, jsonOut);
      Serial.println(jsonOut.c_str());
      mqtt_client.publish("test", jsonOut.c_str());
  }

  mqtt_client.loop();
}

void reconnect(){
  while(!mqtt_client.connected()){
    if(mqtt_client.connect(client_id.c_str())){
      mqtt_client.publish("test", "Hello from ESP32!");
    }else{
      vTaskDelay(1000/portTICK_PERIOD_MS);

    }
  }
}

void ConnectToWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
  
}

void callback(char *topic, byte *payload, unsigned int lenght){

}