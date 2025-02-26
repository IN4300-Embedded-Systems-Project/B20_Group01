#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

// WiFi credentials
const char* ssid = "ESP_Wifi";
const char* password = "12345678";

// ThingSpeak credentials
unsigned long channelID = 2857017;            // Replace with your ThingSpeak channel ID
const char* apiKey = "IWUT7AECP75003F7"; // Replace with your ThingSpeak write API key

// Pin definitions for NodeMCU 1.0 (ESP-12E Module)
const int MOISTURE_SENSOR_PIN = A0;  // Analog pin for the moisture sensor (A0 on NodeMCU)
const int RELAY_PIN = D3;           // Digital pin for relay control (D1 = GPIO5)

// Threshold values
const int MOISTURE_THRESHOLD = 30;  // Adjust based on your sensor calibration (lower = drier)
const unsigned long WATERING_DURATION = 5000;  // Watering duration in milliseconds (5 seconds)
const unsigned long UPDATE_INTERVAL = 60000;   // ThingSpeak update interval (60 seconds)

// Variables
int moistureValue = 0;
int p;
unsigned long lastWateringTime = 0;
unsigned long lastUpdateTime = 0;
WiFiClient client;

void setup() {
  Serial.begin(115200);
  
  // Initialize relay pin as output and ensure pump is off
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  
  // Connect to WiFi
  connectWiFi();
  
  // Initialize ThingSpeak
  ThingSpeak.begin(client);
  
  Serial.println("System initialization completed");
}

void loop() {
  // Read moisture sensor
  readMoistureSensor();
  
  // Check if plant needs watering
  checkAndWaterPlant();
  
  // Update ThingSpeak at regular intervals
  if (millis() - lastUpdateTime > UPDATE_INTERVAL) {
    updateThingSpeak();
    lastUpdateTime = millis();
  }
  
  // Print values to serial monitor
  printSensorValues();
  
  delay(1000);  // Small delay for stability
}

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void readMoistureSensor() {
  // Read the analog value
  // moistureValue = map(analogRead(MOISTURE_SENSOR_PIN),0,1024,1024,0);
  moistureValue = analogRead(MOISTURE_SENSOR_PIN);
  
  // Optional: Convert to percentage (adjust min/max values based on your sensor)
  moistureValue = map(moistureValue, 1023, 0, 0, 100);
  
  Serial.print("Moisture Value: ");
  Serial.println(moistureValue);
}

void checkAndWaterPlant() {
  // Check if soil is dry enough to need watering
  if (moistureValue < MOISTURE_THRESHOLD && (millis() - lastWateringTime > WATERING_DURATION * 2)) {
    Serial.println("Soil is dry. Starting watering cycle...");
    
    // Turn on the pump
    digitalWrite(RELAY_PIN, LOW);
    
    // Wait for the watering duration
    delay(WATERING_DURATION);
    
    // Turn off the pump
    digitalWrite(RELAY_PIN, HIGH);
    
    // Update the last watering time
    lastWateringTime = millis();
    
    Serial.println("Watering cycle completed");
  }
}

void updateThingSpeak() {
  // Set field 1 with the moisture value

  if (moistureValue > MOISTURE_THRESHOLD) {
    p = 0;
  } else {
    p = 1;
  }
  ThingSpeak.setField(1, moistureValue);
  ThingSpeak.setField(2, p);
  
  // Write to the ThingSpeak channel
  int status = ThingSpeak.writeFields(channelID, apiKey);
  
  if (status == 200) {
    Serial.println("ThingSpeak update successful");
  } else {
    Serial.print("ThingSpeak update failed, HTTP error code: ");
    Serial.println(status);
  }
}

void printSensorValues() {
  Serial.println("------- Current Readings -------");
  Serial.print("Moisture Value: ");
  Serial.print(moistureValue);
  Serial.print(" (Threshold: ");
  Serial.print(MOISTURE_THRESHOLD);
  Serial.println(")");
  
  Serial.print("Pump Status: ");
  Serial.println(digitalRead(RELAY_PIN) ? "ON" : "OFF");
  
  Serial.println("-------------------------------");
}