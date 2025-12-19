









#include <Arduino.h>

#include <Korzhenevskiy_tick_tack_2.h>
#include <Korzhenevskiy_utils.h>
#include "secrets.h"
#include <images.h>
#include <math.h>
#include "WiFi.h"
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <AccelStepper.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <Preferences.h>

// ===================================================================================
// PIN DEFINITIONS
// ===================================================================================

#define motor_base_dir 18 // correct
#define motor_base_step 19 // correct
#define motor_base_enable 20 // correct

#define motor_arm_left_dir 15 // correct but not tested
#define motor_arm_left_step 14 // correct but not tested
#define motor_arm_left_enable 8// correct but not tested

#define motor_arm_right_dir 2 // correct but not tested
#define motor_arm_right_step 3 // correct but not tested
#define motor_arm_right_enable 4 // correct but not tested


constexpr int SENSOR_RX_PIN = 1;
constexpr int SENSOR_TX_PIN = -1;
constexpr long SENSOR_BAUD_RATE = 115200;
HardwareSerial& sensorSerial = Serial1;

// ===================================================================================
// MOTOR CONFIGURATION
// ===================================================================================

int motor_base_speed = 400;
int motor_base_accel = motor_base_speed * 2;

int motor_left_speed = 400;
int motor_left_accel = motor_left_speed * 2;

int motor_right_speed = 400;
int motor_right_accel = motor_right_speed * 2;

AccelStepper stepper_base(1, motor_base_step, motor_base_dir);
AccelStepper stepper_left(1, motor_arm_left_step, motor_arm_left_dir);
AccelStepper stepper_right(1, motor_arm_right_step, motor_arm_right_dir);

// ===================================================================================
// DISPLAY AND STORAGE
// ===================================================================================

Adafruit_IS31FL3731 ledmatrix = Adafruit_IS31FL3731();
Preferences preferences;

int currentImage[8][16];
String globalConfigStatus = "";

#define MAX_SAVED_IMAGES 10

// ===================================================================================
// RADAR SENSOR
// ===================================================================================

int rangeValue = 0;
#define HISTORY_SIZE 100
bool humanPresentHistory[HISTORY_SIZE] = {false};
int historyIndex = 0;
bool humanPresent_debounced = false;
bool humanPresent_old = false;

// ===================================================================================
// NETWORK
// ===================================================================================

WiFiClient espClient;
byte mac[6];
IPAddress ip;
PubSubClient client(espClient);

// ===================================================================================
// ANIMATION SYSTEM
// ===================================================================================

#define MAX_ANIMATION_COMMANDS 90

struct AnimationCommand {
  unsigned long t;      // time in ms from animation start
  char motor;           // 'B' = base, 'L' = left, 'R' = right, 'F' = face
  int pos;              // target position (for motors)
  int spd;              // speed (-1 = keep previous)
  int acc;              // acceleration (-1 = keep previous)
  String img;           // image name (for face commands)
};

AnimationCommand animationCommands[MAX_ANIMATION_COMMANDS];
int animationCommandCount = 0;
int animationCurrentIndex = 0;
unsigned long animationStartTime = 0;
bool animationRunning = false;

// ===================================================================================
// FUNCTION DECLARATIONS
// ===================================================================================

void callback(char* topic, byte* payload, unsigned int length);
void check_radar();
void draw_image(const int image[8][16]);
void lookaround();
void read_storage_for_config();
void publish_config_status();
void save_config_to_preferences(String key, int value);
void save_config_to_preferences(String key, bool value);
void saveImageToPreferences(String name);
void loadImageFromPreferences(String name);
void listSavedImages();
void deleteImageFromPreferences(String name);
void parseAnimation(JsonArray arr);
void tickAnimation();
void stopAnimation();

// ===================================================================================
// LOOKAROUND BEHAVIOR
// ===================================================================================

boolean radar_enabled = false;
boolean radar_enabled_temp = radar_enabled;

int lookaround_distance = 200;
int lookaround_status = 0;

Korzhenevskiy_tick_tack_2 timer_lookaround(lookaround, 15000);
Korzhenevskiy_tick_tack_2 timer_delay(lookaround, 4000);

void lookaround() {
  if (lookaround_status == 0) {
    radar_enabled_temp = false;
    stepper_base.moveTo(lookaround_distance);
    draw_image(lookleft);
    lookaround_status++;
    timer_delay.start();
  }
  else if (lookaround_status == 1) {
    stepper_base.moveTo(-lookaround_distance);
    draw_image(lookright);
    lookaround_status++;
    timer_delay.start();
  }
  else if (lookaround_status == 2) {
    stepper_base.moveTo(0);
    draw_image(smileImage);
    lookaround_status = 0;
    timer_delay.stop();
    radar_enabled_temp = radar_enabled;
  }
}

// ===================================================================================
// CONFIG STORAGE FUNCTIONS
// ===================================================================================

void read_storage_for_config() {
  preferences.begin("config", true);
  
  globalConfigStatus = "";
  
  if (preferences.isKey("base_speed")) {
    motor_base_speed = preferences.getInt("base_speed", 400);
    String msg = "base_speed loaded from storage: " + String(motor_base_speed);
    Serial.println(msg);
    globalConfigStatus += msg + "; ";
  } else {
    motor_base_speed = 400;
    String msg = "base_speed not in storage, loaded default value: 400";
    Serial.println(msg);
    globalConfigStatus += msg + "; ";
  }
  motor_base_accel = motor_base_speed * 2;
  stepper_base.setMaxSpeed(motor_base_speed);
  stepper_base.setAcceleration(motor_base_accel);
  
  if (preferences.isKey("left_speed")) {
    motor_left_speed = preferences.getInt("left_speed", 400);
  } else {
    motor_left_speed = 400;
  }
  motor_left_accel = motor_left_speed * 2;
  stepper_left.setMaxSpeed(motor_left_speed);
  stepper_left.setAcceleration(motor_left_accel);
  
  if (preferences.isKey("right_speed")) {
    motor_right_speed = preferences.getInt("right_speed", 400);
  } else {
    motor_right_speed = 400;
  }
  motor_right_accel = motor_right_speed * 2;
  stepper_right.setMaxSpeed(motor_right_speed);
  stepper_right.setAcceleration(motor_right_accel);
  
  int lookaround_delay_val;
  if (preferences.isKey("lookaround_delay")) {
    lookaround_delay_val = preferences.getInt("lookaround_delay", 15000);
    String msg = "lookaround_delay loaded from storage: " + String(lookaround_delay_val);
    Serial.println(msg);
    globalConfigStatus += msg + "; ";
  } else {
    lookaround_delay_val = 15000;
    String msg = "lookaround_delay not in storage, loaded default value: 15000";
    Serial.println(msg);
    globalConfigStatus += msg + "; ";
  }
  timer_lookaround.setinterval(lookaround_delay_val);
  
  if (preferences.isKey("lookaround_distance")) {
    lookaround_distance = preferences.getInt("lookaround_distance", 200);
    String msg = "lookaround_distance loaded from storage: " + String(lookaround_distance);
    Serial.println(msg);
    globalConfigStatus += msg + "; ";
  } else {
    lookaround_distance = 200;
    String msg = "lookaround_distance not in storage, loaded default value: 200";
    Serial.println(msg);
    globalConfigStatus += msg + "; ";
  }
  
  boolean lookaround_enable = false;
  if (preferences.isKey("lookaround_enable")) {
    lookaround_enable = preferences.getBool("lookaround_enable", false);
    String msg = "lookaround_enable loaded from storage: " + String(lookaround_enable ? "true" : "false");
    Serial.println(msg);
    globalConfigStatus += msg + "; ";
  } else {
    lookaround_enable = false;
    String msg = "lookaround_enable not in storage, loaded default value: false";
    Serial.println(msg);
    globalConfigStatus += msg + "; ";
  }
  
  timer_lookaround.stop();
  
  preferences.end();
  
  Serial.println("Configuration loading complete");
}

void publish_config_status() {
  Serial.println("publish_config_status");
  String mqttMessage = "{\"config_status\": \"" + globalConfigStatus + "\"}";
  client.publish(MQTTChannelToPublish, mqttMessage.c_str());
  Serial.println("Config status published to MQTT");
}

void save_config_to_preferences(String key, int value) {
  preferences.begin("config", false);
  preferences.putInt(key.c_str(), value);
  preferences.end();
  Serial.print("Saved config: ");
  Serial.print(key);
  Serial.print(" = ");
  Serial.println(value);
}

void save_config_to_preferences(String key, bool value) {
  preferences.begin("config", false);
  preferences.putBool(key.c_str(), value);
  preferences.end();
  Serial.print("Saved config: ");
  Serial.print(key);
  Serial.print(" = ");
  Serial.println(value ? "true" : "false");
}

// ===================================================================================
// IMAGE STORAGE FUNCTIONS
// ===================================================================================

void saveImageToPreferences(String name) {
  if (name.length() == 0 || name.length() > 15) {
    Serial.println("Error: Image name must be 1-15 characters");
    client.publish(MQTTChannelToPublish, "{\"error\": \"Image name must be 1-15 characters\"}");
    return;
  }

  preferences.begin("images", false);
  
  String imageList = preferences.getString("image_list", "");
  
  int nameCount = 0;
  if (imageList.length() > 0) {
    int startPos = 0;
    while (startPos < (int)imageList.length()) {
      int endPos = imageList.indexOf(',', startPos);
      if (endPos == -1) endPos = imageList.length();
      nameCount++;
      startPos = endPos + 1;
    }
  }
  
  bool nameExists = false;
  if (imageList.indexOf(name) != -1) {
    int startPos = 0;
    while (startPos < (int)imageList.length()) {
      int endPos = imageList.indexOf(',', startPos);
      if (endPos == -1) endPos = imageList.length();
      String existingName = imageList.substring(startPos, endPos);
      if (existingName.equals(name)) {
        nameExists = true;
        break;
      }
      startPos = endPos + 1;
    }
  }
  
  if (!nameExists && nameCount >= MAX_SAVED_IMAGES) {
    Serial.println("Error: Maximum number of saved images reached");
    client.publish(MQTTChannelToPublish, "{\"error\": \"Maximum number of saved images reached\"}");
    preferences.end();
    return;
  }
  
  uint8_t imageData[16];
  for (int byteIdx = 0; byteIdx < 16; byteIdx++) {
    imageData[byteIdx] = 0;
    for (int bitIdx = 0; bitIdx < 8; bitIdx++) {
      int row = byteIdx / 2;
      int col = (byteIdx % 2) * 8 + bitIdx;
      if (currentImage[row][col] == 1) {
        imageData[byteIdx] |= (1 << bitIdx);
      }
    }
  }
  
  preferences.putBytes(name.c_str(), imageData, 16);
  
  if (!nameExists) {
    if (imageList.length() > 0) {
      imageList += ",";
    }
    imageList += name;
    preferences.putString("image_list", imageList);
  }
  
  preferences.end();
  
  Serial.print("Image saved: ");
  Serial.println(name);
  
  String response = "{\"image_saved\": \"" + name + "\"}";
  client.publish(MQTTChannelToPublish, response.c_str());
}

void loadImageFromPreferences(String name) {
  preferences.begin("images", true);
  
  if (!preferences.isKey(name.c_str())) {
    Serial.print("Error: Image not found: ");
    Serial.println(name);
    String response = "{\"error\": \"Image not found: " + name + "\"}";
    client.publish(MQTTChannelToPublish, response.c_str());
    preferences.end();
    return;
  }
  
  uint8_t imageData[16];
  size_t len = preferences.getBytes(name.c_str(), imageData, 16);
  
  if (len != 16) {
    Serial.println("Error: Corrupted image data");
    client.publish(MQTTChannelToPublish, "{\"error\": \"Corrupted image data\"}");
    preferences.end();
    return;
  }
  
  preferences.end();
  
  int loadedImage[8][16];
  for (int byteIdx = 0; byteIdx < 16; byteIdx++) {
    for (int bitIdx = 0; bitIdx < 8; bitIdx++) {
      int row = byteIdx / 2;
      int col = (byteIdx % 2) * 8 + bitIdx;
      loadedImage[row][col] = (imageData[byteIdx] & (1 << bitIdx)) ? 1 : 0;
    }
  }
  
  draw_image(loadedImage);
  
  Serial.print("Image loaded and displayed: ");
  Serial.println(name);
  
  String response = "{\"image_loaded\": \"" + name + "\"}";
  client.publish(MQTTChannelToPublish, response.c_str());
}

void listSavedImages() {
  preferences.begin("images", true);
  
  String imageList = preferences.getString("image_list", "");
  preferences.end();
  
  if (imageList.length() == 0) {
    client.publish(MQTTChannelToPublish, "{\"saved_images\": []}");
    Serial.println("No saved images");
    return;
  }
  
  String jsonResponse = "{\"saved_images\": [";
  int startPos = 0;
  bool first = true;
  
  while (startPos < (int)imageList.length()) {
    int endPos = imageList.indexOf(',', startPos);
    if (endPos == -1) endPos = imageList.length();
    
    String imageName = imageList.substring(startPos, endPos);
    
    if (!first) {
      jsonResponse += ", ";
    }
    jsonResponse += "\"" + imageName + "\"";
    first = false;
    
    startPos = endPos + 1;
  }
  
  jsonResponse += "]}";
  
  Serial.print("Saved images: ");
  Serial.println(jsonResponse);
  
  client.publish(MQTTChannelToPublish, jsonResponse.c_str());
}

void deleteImageFromPreferences(String name) {
  preferences.begin("images", false);
  
  if (!preferences.isKey(name.c_str())) {
    Serial.print("Error: Image not found: ");
    Serial.println(name);
    preferences.end();
    return;
  }
  
  preferences.remove(name.c_str());
  
  String imageList = preferences.getString("image_list", "");
  String newList = "";
  int startPos = 0;
  bool first = true;
  
  while (startPos < (int)imageList.length()) {
    int endPos = imageList.indexOf(',', startPos);
    if (endPos == -1) endPos = imageList.length();
    
    String imageName = imageList.substring(startPos, endPos);
    
    if (!imageName.equals(name)) {
      if (!first) {
        newList += ",";
      }
      newList += imageName;
      first = false;
    }
    
    startPos = endPos + 1;
  }
  
  preferences.putString("image_list", newList);
  preferences.end();
  
  Serial.print("Image deleted: ");
  Serial.println(name);
  
  String response = "{\"image_deleted\": \"" + name + "\"}";
  client.publish(MQTTChannelToPublish, response.c_str());
}

// ===================================================================================
// ANIMATION SYSTEM FUNCTIONS
// ===================================================================================

void parseAnimation(JsonArray arr) {
  animationCommandCount = 0;
  
  for (JsonObject cmd : arr) {
    if (animationCommandCount >= MAX_ANIMATION_COMMANDS) {
      Serial.println("Warning: Animation truncated, too many commands");
      break;
    }
    
    AnimationCommand& ac = animationCommands[animationCommandCount];
    
    ac.t = cmd["ms"].as<unsigned long>();
    
    // Check for screen command first
    if (cmd.containsKey("screen")) {
      ac.motor = 'F';
      ac.img = cmd["screen"].as<String>();
      ac.pos = 0;
      ac.spd = -1;
      ac.acc = -1;
    }
    // Otherwise it's a motor command
    else if (cmd.containsKey("motor")) {
      String motorStr = cmd["motor"].as<String>();
      if (motorStr == "base") {
        ac.motor = 'B';
      } else if (motorStr == "left") {
        ac.motor = 'L';
      } else if (motorStr == "right") {
        ac.motor = 'R';
      } else {
        Serial.print("Warning: Unknown motor: ");
        Serial.println(motorStr);
        continue;
      }
      ac.pos = cmd["pos"].as<int>();
      ac.spd = cmd.containsKey("spd") ? cmd["spd"].as<int>() : -1;
      ac.acc = cmd.containsKey("acc") ? cmd["acc"].as<int>() : -1;
      ac.img = "";
    }
    else {
      Serial.println("Warning: Command missing 'motor' or 'screen' key");
      continue;
    }
    
    animationCommandCount++;
  }
  
  Serial.print("Animation parsed, ");
  Serial.print(animationCommandCount);
  Serial.println(" commands");
  
  // Start animation
  animationCurrentIndex = 0;
  animationStartTime = millis();
  animationRunning = true;
  
  String response = "{\"animation_started\": true, \"commands\": " + String(animationCommandCount) + "}";
  client.publish(MQTTChannelToPublish, response.c_str());
}
void tickAnimation() {
  if (!animationRunning) return;
  
  unsigned long elapsed = millis() - animationStartTime;
  
  // Process all commands whose time has come
  while (animationCurrentIndex < animationCommandCount) {
    AnimationCommand& ac = animationCommands[animationCurrentIndex];
    
    if (elapsed < ac.t) {
      // Not yet time for this command
      break;
    }
    
    // Execute this command
    if (ac.motor == 'F') {
      // Face command - load image from preferences
      loadImageFromPreferences(ac.img);
      Serial.print("Animation: face -> ");
      Serial.println(ac.img);
    }
    else {
      // Motor command
      AccelStepper* stepper = nullptr;
      int* currentSpeed = nullptr;
      int* currentAccel = nullptr;
      
      if (ac.motor == 'B') {
        stepper = &stepper_base;
        currentSpeed = &motor_base_speed;
        currentAccel = &motor_base_accel;
      } else if (ac.motor == 'L') {
        stepper = &stepper_left;
        currentSpeed = &motor_left_speed;
        currentAccel = &motor_left_accel;
      } else if (ac.motor == 'R') {
        stepper = &stepper_right;
        currentSpeed = &motor_right_speed;
        currentAccel = &motor_right_accel;
      }
      
      if (stepper != nullptr) {
        // Update speed if provided
        if (ac.spd > 0) {
          *currentSpeed = ac.spd;
          stepper->setMaxSpeed(ac.spd);
        }
        
        // Update acceleration if provided
        if (ac.acc > 0) {
          *currentAccel = ac.acc;
          stepper->setAcceleration(ac.acc);
        }
        
        // Command the move
        stepper->moveTo(ac.pos);
        
        Serial.print("Animation: motor ");
        Serial.print(ac.motor);
        Serial.print(" -> ");
        Serial.print(ac.pos);
        Serial.print(" spd=");
        Serial.print(*currentSpeed);
        Serial.print(" acc=");
        Serial.println(*currentAccel);
      }
    }
    
    animationCurrentIndex++;
  }
  
  // Check if animation is complete
if (animationCurrentIndex >= animationCommandCount) {
  // All commands dispatched - wait for motors to finish
  if (!stepper_base.isRunning() && !stepper_left.isRunning() && !stepper_right.isRunning()) {
    animationRunning = false;
    
    // Normalize base position after full rotations
    long pos = stepper_base.currentPosition();
    if (pos >= 5560 || pos <= -5560) {
      stepper_base.setCurrentPosition(pos % 5560);
    }
    
    Serial.println("Animation complete");
    client.publish(MQTTChannelToPublish, "{\"animation_complete\": true}");
    }
  }
}

void stopAnimation() {
  animationRunning = false;
  stepper_base.stop();
  stepper_left.stop();
  stepper_right.stop();
  Serial.println("Animation stopped");
  client.publish(MQTTChannelToPublish, "{\"animation_stopped\": true}");
}

// ===================================================================================
// SETUP
// ===================================================================================

void setup() {
  // Motor pin modes
  pinMode(motor_base_dir, OUTPUT);
  pinMode(motor_base_step, OUTPUT);
  pinMode(motor_base_enable, OUTPUT);
  
  pinMode(motor_arm_left_dir, OUTPUT);
  pinMode(motor_arm_left_step, OUTPUT);
  pinMode(motor_arm_left_enable, OUTPUT);
  
  pinMode(motor_arm_right_dir, OUTPUT);
  pinMode(motor_arm_right_step, OUTPUT);
  pinMode(motor_arm_right_enable, OUTPUT);
  
  // Disable motors initially (HIGH = disabled for TMC2209)
  digitalWrite(motor_base_enable, HIGH);
  digitalWrite(motor_arm_left_enable, HIGH);
  digitalWrite(motor_arm_right_enable, HIGH);
  
  Serial.begin(115200);
  sensorSerial.begin(SENSOR_BAUD_RATE, SERIAL_8N1, SENSOR_RX_PIN, SENSOR_TX_PIN);
  
  Wire.begin(7, 6);
  delay(100);
  
  if (!ledmatrix.begin()) {
    Serial.println("IS31 not found");
    while (1);
  }
  Serial.println("IS31 found!");

  client.setBufferSize(4096);
  client.setServer(mqtt_server, mqttport);
  client.setCallback(callback);
  
  draw_cat();
  connecttobestwifi(networks, numNetworks);
  connecttoMQTT();
  
  read_storage_for_config();
  
  // Initialize all steppers
  stepper_base.setMaxSpeed(motor_base_speed);
  stepper_base.setAcceleration(motor_base_accel);
  stepper_base.setCurrentPosition(0);
  
  stepper_left.setMaxSpeed(motor_left_speed);
  stepper_left.setAcceleration(motor_left_accel);
  stepper_left.setCurrentPosition(0);
  
  stepper_right.setMaxSpeed(motor_right_speed);
  stepper_right.setAcceleration(motor_right_accel);
  stepper_right.setCurrentPosition(0);
  
  publish_config_status();
  
  draw_image(smileImage);
}

// ===================================================================================
// LOOP
// ===================================================================================

void loop() {
  // Motor enable control - enable only when running
  if (stepper_base.isRunning()) digitalWrite(motor_base_enable, LOW);
  else digitalWrite(motor_base_enable, HIGH);
  
  if (stepper_left.isRunning()) digitalWrite(motor_arm_left_enable, LOW);
  else digitalWrite(motor_arm_left_enable, HIGH);
  
  if (stepper_right.isRunning()) digitalWrite(motor_arm_right_enable, LOW);
  else digitalWrite(motor_arm_right_enable, HIGH);
  
  // Run all steppers
  stepper_base.run();
  stepper_left.run();
  stepper_right.run();
  
  // Tick animation system
  tickAnimation();
  
  // Tick timers
  Korzhenevskiy_tick_tack_2::mass_tick();
  
  // Check radar if enabled
  if ((radar_enabled) && (radar_enabled_temp)) check_radar();
  
  // Human detection state changes
  if ((humanPresent_debounced) && (!humanPresent_old)) {
    draw_image(smileImage);
    client.publish(MQTTChannelToPublish, "{\"human\": 1}");
    Serial.println("human detected");
  }
  
  if ((!humanPresent_debounced) && (humanPresent_old)) {
    ledmatrix.clear();
    client.publish(MQTTChannelToPublish, "{\"human\": 0}");
    Serial.println("human is gone");
  }
  
  humanPresent_old = humanPresent_debounced;
  
  // MQTT reconnection
  if (!client.connected()) {
    connecttoMQTT();
  }
  client.loop();
}

// ===================================================================================
// MQTT CALLBACK
// ===================================================================================

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  
  for (unsigned int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.print("New message from MQTT: ");
  Serial.println(message);
  
  DynamicJsonDocument doc(4096);  // Increased size for animations
  
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }
  
  Serial.println("JSON parse success");
  
  // ---------------------------------------------------------------------------------
  // ANIMATION COMMAND
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("animation")) {
    JsonArray arr = doc["animation"].as<JsonArray>();
    parseAnimation(arr);
    return;  // Animation takes priority, ignore other commands in same message
  }
  
  // ---------------------------------------------------------------------------------
  // ANIMATION STOP
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("animation_stop")) {
    stopAnimation();
  }
  
  // ---------------------------------------------------------------------------------
  // SYSTEM COMMANDS
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("command")) {
    String command = doc["command"];
    if (command == "reset") ESP.restart();
  }
  
  // ---------------------------------------------------------------------------------
  // IMAGE COMMANDS
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("image")) {
    String imageString = doc["image"];
    Serial.println("Image received:");
    Serial.println(imageString);
    int Image[8][16];
    if (imageString.length() == 128) {
      int index = 0;
      for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 16; col++) {
          Image[row][col] = imageString.charAt(index) - '0';
          index++;
        }
      }
      Serial.println("Image array populated successfully");
      draw_image(Image);
    } else {
      Serial.println("Error: Image string length is not 128");
    }
  }
  
  if (doc.containsKey("image_save")) {
    String imageName = doc["image_save"].as<String>();
    saveImageToPreferences(imageName);
  }
  
  if (doc.containsKey("image_show")) {
    String imageName = doc["image_show"].as<String>();
    loadImageFromPreferences(imageName);
  }
  
  if (doc.containsKey("image_show_all")) {
    listSavedImages();
  }
  
  if (doc.containsKey("image_delete")) {
    String imageName = doc["image_delete"].as<String>();
    deleteImageFromPreferences(imageName);
  }
  
  // ---------------------------------------------------------------------------------
  // RADAR CONTROL
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("radarenable")) {
    int radarValue = doc["radarenable"];
    radar_enabled = (radarValue == 1);
    Serial.print("Radar enabled set to: ");
    Serial.println(radar_enabled ? "true" : "false");
  }
  
  // ---------------------------------------------------------------------------------
  // LOOKAROUND CONTROL
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("lookaround_enable")) {
    int Value = doc["lookaround_enable"];
    boolean lookaround_enable = (Value == 1);
    if (lookaround_enable) timer_lookaround.start();
    else timer_lookaround.stop();
    save_config_to_preferences("lookaround_enable", lookaround_enable);
    Serial.print("lookaround_enable set to: ");
    Serial.println(lookaround_enable ? "true" : "false");
  }
  
  if (doc.containsKey("lookaround_enable_once")) {
    lookaround();
  }
  
  if (doc.containsKey("lookaround_delay")) {
    int Value = doc["lookaround_delay"];
    timer_lookaround.setinterval(Value * 1000);
    save_config_to_preferences("lookaround_delay", Value * 1000);
    Serial.print("lookaround_delay ");
    Serial.println(Value * 1000);
  }
  
  if (doc.containsKey("lookaround_distance")) {
    int Value = doc["lookaround_distance"];
    lookaround_distance = Value;
    save_config_to_preferences("lookaround_distance", Value);
    Serial.print("lookaround_distance ");
    Serial.println(Value);
  }
  
  // ---------------------------------------------------------------------------------
  // BASE MOTOR DIRECT CONTROL
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("base_move")) {
    int Value = doc["base_move"];
   
      stepper_base.moveTo(Value);
    
  }
  
  if (doc.containsKey("base_zero")) {
    stepper_base.setCurrentPosition(0);
    Serial.println("base set to home");
  }
  
  if (doc.containsKey("base_speed")) {
    int Value = doc["base_speed"];
    motor_base_speed = Value;
    motor_base_accel = Value * 2;
    stepper_base.setMaxSpeed(Value);
    stepper_base.setAcceleration(motor_base_accel);
    save_config_to_preferences("base_speed", Value);
    Serial.print("base speed/accel changed: ");
    Serial.println(Value);
  }
  
  if (doc.containsKey("base_acc")) {
    int Value = doc["base_acc"];
    motor_base_accel = Value;
    stepper_base.setAcceleration(Value);
    Serial.print("base acceleration changed to ");
    Serial.println(Value);
  }
  
  // ---------------------------------------------------------------------------------
  // LEFT ARM MOTOR DIRECT CONTROL
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("left_move")) {
    int Value = doc["left_move"];
    if (Value >= -1000 && Value <= 1000) {
      stepper_left.moveTo(Value);
    }
  }
  
  if (doc.containsKey("left_zero")) {
    stepper_left.setCurrentPosition(0);
    Serial.println("left arm set to home");
  }
  
  if (doc.containsKey("left_speed")) {
    int Value = doc["left_speed"];
    motor_left_speed = Value;
    motor_left_accel = Value * 2;
    stepper_left.setMaxSpeed(Value);
    stepper_left.setAcceleration(motor_left_accel);
    save_config_to_preferences("left_speed", Value);
    Serial.print("left speed/accel changed: ");
    Serial.println(Value);
  }
  
  if (doc.containsKey("left_acc")) {
    int Value = doc["left_acc"];
    motor_left_accel = Value;
    stepper_left.setAcceleration(Value);
    Serial.print("left acceleration changed to ");
    Serial.println(Value);
  }
  
  // ---------------------------------------------------------------------------------
  // RIGHT ARM MOTOR DIRECT CONTROL
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("right_move")) {
    int Value = doc["right_move"];
    if (Value >= -1000 && Value <= 1000) {
      stepper_right.moveTo(Value);
    }
  }
  
  if (doc.containsKey("right_zero")) {
    stepper_right.setCurrentPosition(0);
    Serial.println("right arm set to home");
  }
  
  if (doc.containsKey("right_speed")) {
    int Value = doc["right_speed"];
    motor_right_speed = Value;
    motor_right_accel = Value * 2;
    stepper_right.setMaxSpeed(Value);
    stepper_right.setAcceleration(motor_right_accel);
    save_config_to_preferences("right_speed", Value);
    Serial.print("right speed/accel changed: ");
    Serial.println(Value);
  }
  
  if (doc.containsKey("right_acc")) {
    int Value = doc["right_acc"];
    motor_right_accel = Value;
    stepper_right.setAcceleration(Value);
    Serial.print("right acceleration changed to ");
    Serial.println(Value);
  }
  
  // ---------------------------------------------------------------------------------
  // ALL MOTORS ZERO
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("all_zero")) {
    stepper_base.setCurrentPosition(0);
    stepper_left.setCurrentPosition(0);
    stepper_right.setCurrentPosition(0);
    Serial.println("all motors set to home");
    client.publish(MQTTChannelToPublish, "{\"all_zero\": true}");
  }
  
  // ---------------------------------------------------------------------------------
  // ALL MOTORS HOME (move to 0)
  // ---------------------------------------------------------------------------------
  if (doc.containsKey("all_home")) {
    stepper_base.moveTo(0);
    stepper_left.moveTo(0);
    stepper_right.moveTo(0);
    Serial.println("all motors moving to home");
    client.publish(MQTTChannelToPublish, "{\"all_home\": true}");
  }
// ---------------------------------------------------------------------------------
// JOG COMMANDS (relative movement)
// ---------------------------------------------------------------------------------
if (doc.containsKey("base_jog")) {
  int Value = doc["base_jog"];
  long current = stepper_base.currentPosition();
  stepper_base.moveTo(current + Value);
  Serial.print("base jog: ");
  Serial.println(current + Value);
}

if (doc.containsKey("left_jog")) {
  int Value = doc["left_jog"];
  long current = stepper_left.currentPosition();
  stepper_left.moveTo(current + Value);
  Serial.print("left jog: ");
  Serial.println(current + Value);
}

if (doc.containsKey("right_jog")) {
  int Value = doc["right_jog"];
  long current = stepper_right.currentPosition();
  stepper_right.moveTo(current + Value);
  Serial.print("right jog: ");
  Serial.println(current + Value);
}

}

// ===================================================================================
// RADAR CHECK
// ===================================================================================

void check_radar(void) {
  if (sensorSerial.available()) {
    String receivedString = sensorSerial.readStringUntil('\n');
    receivedString.trim();
    
    if (receivedString.length() > 0) {
      if (receivedString.equals("ON") || receivedString.equals("OFF")) {
        bool currentReading = receivedString.equals("ON");
        humanPresentHistory[historyIndex] = currentReading;
        historyIndex = (historyIndex + 1) % HISTORY_SIZE;
        
        bool allFalse = true;
        for (int i = 0; i < HISTORY_SIZE; i++) {
          if (humanPresentHistory[i]) {
            allFalse = false;
            break;
          }
        }
        
        if (currentReading) {
          humanPresent_debounced = true;
        } else if (allFalse) {
          humanPresent_debounced = false;
        }
      }
    }
  }
}

// ===================================================================================
// DRAW IMAGE
// ===================================================================================

void draw_image(const int image[8][16]) {
  // Copy to currentImage global variable
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 16; j++) {
      currentImage[i][j] = image[i][j];
    }
  }
  
  // Transform for display
  int image_to_display[16][8];
  
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      image_to_display[i][j] = image[i][j];
    }
  }
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      image_to_display[i + 8][j] = image[i][j + 8];
    }
  }
  
  // Draw
  ledmatrix.clear();
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 16; x++) {
      if (image_to_display[x][y] == 1) {
        ledmatrix.drawPixel(x, y + 1, 255);
      }
    }
  }
}