#include "Korzhenevskiy_utils.h"

void draw_cat(void) {

  Serial.println("  ");
  Serial.println("  ");
  Serial.println("  ");
  Serial.println("  ");
  Serial.println("   xx                xx");
  Serial.println("   xxxx            xxxx                 xxxxxxx");
  Serial.println("  x   xxxxxxxxxxxxxx  xx             xxxx     xxxxxxx");
  Serial.println(" xx                    x             xx              xxx");
  Serial.println("xx   xxxx     xxxxxx   xx             xxxxxxxxxxxx     xx");
  Serial.println("x  xxx  xx    x    xx   x                        xxx    xx");
  Serial.println("x   xxxxxx    xxxxxxx   x                       xxx      x");
  Serial.println("xxx                     xxxxxxxxxxxxxxxxxxxxxxxxx        x");
  Serial.println("  xxxxxxx        xxxxxxxx                xxx             x");
  Serial.println("        xxxxxxxxxx  x                   xx               x");
  Serial.println("                    x                   xx               x");
  Serial.println("                    x                    x               x");
  Serial.println("              xxxxxx             xxxxxxxxx              xx");
  Serial.println("             x                  xx                 xxxxx");
  Serial.println("             xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
  Serial.println("  ");
  Serial.println("  ");
  Serial.println("  ");
  Serial.println("  ");
}

void connecttobestwifi(wificredentials networks[], int numNetworks) {
    // Your existing connecttobestwifi() function code.
    // The variables 'networks' and 'numNetworks' are now available
    // because they are passed as parameters.
    // You must also declare 'ip' as a local variable.

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("Scan start");
    int n = WiFi.scanNetworks();
    Serial.println("Scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            for (int i3 = 0; i3 < numNetworks; i3++) {
                if (WiFi.SSID(i) == networks[i3].ssid) {
                    networks[i3].power = WiFi.RSSI(i);
                    Serial.print("found ");
                    Serial.print(networks[i3].ssid);
                    Serial.print(" ");
                    Serial.println(networks[i3].power);
                }
            }
        }
    }
    
    int bestNetworkIndex = -1;
    int maxPower = -500;
    
    for (int i = 0; i < numNetworks; i++) {
        if (networks[i].power > maxPower) {
            maxPower = networks[i].power;
            bestNetworkIndex = i;
        }
    }
    
    if (bestNetworkIndex != -1) {
        Serial.print("Connecting to: ");
        Serial.println(networks[bestNetworkIndex].ssid);
        WiFi.begin(networks[bestNetworkIndex].ssid, networks[bestNetworkIndex].pass);
        while (WiFi.status() != WL_CONNECTED) {
            delay(100);
            Serial.print(".");
        }
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        IPAddress ip = WiFi.localIP(); // Declare 'ip' here as a local variable
        Serial.println(ip);
    }
}




void connecttoMQTT() {
  if (!client.connected()) {  // CHANDED FOM WHILE
    Serial.println("Attempting MQTT connection...");

    //boolean connect (clientID, username, password, willTopic, willQoS, willRetain, willMessage)
    if (client.connect(MQTTclientname, mqtt_user, mqtt_password, MQTTChannelToPublish, 1, false, willMessage)) {
      Serial.println("MQTT connected");
      client.subscribe(MQTTChannelToSubsribe);
      client.publish(MQTTChannelToPublish, HelloMessage);


    } else {
      Serial.println("MQTT connect fail !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ");
      
    }
  }
}
