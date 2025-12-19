#ifndef KORZHENEVSKIY_UTILS_H
#define KORZHENEVSKIY_UTILS_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

struct wificredentials {
  const char* ssid;
  const char* pass;
  int power;
};


extern PubSubClient client;

extern const char* mqtt_server;
extern const char* mqtt_user;
extern const char* mqtt_password;
extern const int mqttport;
extern const char* MQTTclientname;
extern const char* willMessage;
extern const char* HelloMessage;
extern const char* MQTTChannelToPublish;
extern const char* MQTTChannelToSubsribe;
extern wificredentials networks[];
extern const int numNetworks; 

void connecttobestwifi(wificredentials networks[], int numNetworks);
void connecttoMQTT();
void draw_cat();

#endif // KORZHENEVSKIY_UTILS_H