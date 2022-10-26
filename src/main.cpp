#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Secrets.h"

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

#pragma pack(1)
typedef struct Payloads {
    std::uint16_t moist_1;
    std::uint16_t moist_2;
    float         temp;
    float         hum;
}Payload, * PPayload;
#pragma pack()

std::uint8_t * buffer;


void ReadSerial(void * parameter){
  while (1)
  {
    if(Serial2.available() > 0){
      Serial.readBytes(buffer,sizeof(Payloads));
      std::uint8_t  * p = &buffer[0];
      for(int i = 0; i < sizeof(Payloads); i++){
        Serial.println(*p);
        p++;
      }
      
    }
  }
  
}

void setup() {
  Serial.begin(9600);
  Serial.println("hello");
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);

  xTaskCreate(
    ReadSerial,
    "Serial_task",
    1000,
    NULL,
    1,
    NULL);

  
  ConnectToWiFi();
  mqtt_client.setServer(mqtt_host, mqtt_port);
  mqtt_client.setCallback(callback);
}


void loop() {

  if(!mqtt_client.connected()){
    reconnect();
  }

  mqtt_client.loop();
}

void reconnect(){
  while(!mqtt_client.connected()){
    if(mqtt_client.connect(client_id.c_str())){
      mqtt_client.publish("test", "Hello from ESP32!");
    }else{
      delay(500);
    }
  }
}

void ConnectToWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  
}

void callback(char *topic, byte *payload, unsigned int lenght){

}