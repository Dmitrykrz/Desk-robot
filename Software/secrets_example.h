
#ifndef SECRETS_H
#define SECRETS_H

#include "Korzhenevskiy_utils.h" 

// --- MQTT Configuration ---
const char* mqtt_server = "";
const char* mqtt_user = "";
const char* mqtt_password = "";
const int mqttport = ;
const char* willMessage = "";
const char* HelloMessage = "";
const char* MQTTclientname = "";
const char* MQTTChannelToPublish = "";
const char* MQTTChannelToSubsribe = "";

// --- WiFi Credentials ---
wificredentials networks[] = {
  {"your_ssid_1", "your_pass_1", -500},
  {"your_ssid_2", "your_pass_2", -500},
  {"your_ssid_3", "your_pass_3", -500}
};
const int numNetworks = sizeof(networks) / sizeof(networks[0]);

#endif // SECRETS_H

