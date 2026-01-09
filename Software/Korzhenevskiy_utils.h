#ifndef KORZHENEVSKIY_UTILS_H
#define KORZHENEVSKIY_UTILS_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
//#include <Adafruit_IS31FL3731.h>

// The struct for WiFi credentials must be defined first
struct wificredentials {
  const char* ssid;
  const char* pass;
  int power;
};

// Declare external variables used in the utility functions
extern PubSubClient client;
//extern Adafruit_IS31FL3731 ledmatrix;

// Extern declarations for global constants and variables defined in secrets.h
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
extern const int numNetworks; // To get the size of the networks array

// Declare the function prototypes
void connecttobestwifi(wificredentials networks[], int numNetworks);
void connecttoMQTT();
void draw_cat();

#endif // KORZHENEVSKIY_UTILS_H