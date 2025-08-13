#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>

#define SIM800_RX 16
#define SIM800_TX 17
#define GPS_RX 26
#define GPS_TX 19
#define MIC_PIN 34
#define LED_PIN 2

HardwareSerial sim800Serial(2);
HardwareSerial gpsSerial(1);
TinyGPSPlus gps;
WiFiClientSecure client;

String WIFI_SSID;
String WIFI_PASSWORD;
String PERSON_NAME;
String PLACE_NAME;
String PHONE_NUMBER;
String SUPABASE_URL;
String SUPABASE_ANON_KEY;

const int NOISE_THRESHOLD = 100;
const int EMERGENCY_THRESHOLD = 400;
const int CONFIDENCE_THRESHOLD = 75;

unsigned long lastAlertTime = 0;
unsigned long lastVoiceCheck = 0;
const unsigned long VOICE_CHECK_INTERVAL = 3000;
const unsigned long EMERGENCY_INTERVAL = 50000;
int alertCounter = 1;

float ambientNoiseLevel = 0;
int voiceDetectionCount = 0;
bool systemCalibrated = false;

double currentLatitude = 0;
double currentLongitude = 0;
bool gpsLockAcquired = false;

String emergencyKeywords[] = {"danger", "fire", "snake"};
String threatPatterns[] = {"danger_alert", "fire_emergency", "snake_attack"};

void setup() {
  Serial.begin(115200);
  delay(2000);
  requestConfig();
  setupHardware();
  setupWiFi();
  calibrateVoiceDetection();
}

void loop() {
  updateGPSLocation();
  if (millis() - lastVoiceCheck > VOICE_CHECK_INTERVAL) {
    checkVoiceActivity();
    lastVoiceCheck = millis();
  }
  if (millis() - lastAlertTime >= EMERGENCY_INTERVAL) {
    simulateEmergencyDetection();
    lastAlertTime = millis();
  }
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 's' || command == 'S') {
      showSystemStatus();
    }
  }
  delay(100);
}

void requestConfig() {
  Serial.println("Enter WiFi SSID:");
  WIFI_SSID = readSerialLine();
  Serial.println("Enter WiFi Password:");
  WIFI_PASSWORD = readSerialLine();
  Serial.println("Enter Person Name:");
  PERSON_NAME = readSerialLine();
  Serial.println("Enter Place Name:");
  PLACE_NAME = readSerialLine();
  Serial.println("Enter Phone Number:");
  PHONE_NUMBER = readSerialLine();
  Serial.println("Enter Supabase URL:");
  SUPABASE_URL = readSerialLine();
  Serial.println("Enter Supabase Anon Key:");
  SUPABASE_ANON_KEY = readSerialLine();
}

String readSerialLine() {
  String input = "";
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n') break;
      if (c != '\r') input += c;
    }
  }
  return input;
}

void setupHardware() {
  pinMode(MIC_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  sim800Serial.begin(9600, SERIAL_8N1, SIM800_RX, SIM800_TX);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
}

void setupWiFi() {
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) client.setInsecure();
}

void calibrateVoiceDetection() {
  long totalNoise = 0;
  int samples = 0;
  unsigned long calibrationStart = millis();
  while (millis() - calibrationStart < 3000) {
    int noiseLevel = analogRead(MIC_PIN);
    totalNoise += noiseLevel;
    samples++;
    delay(50);
  }
  ambientNoiseLevel = totalNoise / samples;
  systemCalibrated = true;
}

void updateGPSLocation() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        currentLatitude = gps.location.lat();
        currentLongitude = gps.location.lng();
        gpsLockAcquired = true;
      }
    }
  }
}

void checkVoiceActivity() {
  if (!systemCalibrated) return;
  int micValue = analogRead(MIC_PIN);
}

void simulateEmergencyDetection() {
  int emergencyIndex = random(0, 3);
  String detectedKeyword = emergencyKeywords[emergencyIndex];
  String threatType = threatPatterns[emergencyIndex];
  int confidenceScore = random(85, 96);
  triggerEmergencyAlert(detectedKeyword, confidenceScore, threatType);
}

void triggerEmergencyAlert(String keyword, int confidence, String threatType) {
  if (WiFi.status() == WL_CONNECTED) {
    postToSupabase(keyword, currentLatitude, currentLongitude, "emergency", confidence, threatType);
  } else {
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
    delay(2000);
  }
  alertCounter++;
}

void showSystemStatus() {
  Serial.printf("Uptime: %lu sec\n", millis() / 1000);
  Serial.printf("Emergency Alerts: %d\n", alertCounter - 1);
  Serial.printf("Voice Detections: %d\n", voiceDetectionCount);
  Serial.printf("WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Offline");
  Serial.printf("GPS: %s (%.6f, %.6f)\n", gpsLockAcquired ? "LOCKED" : "ACQUIRING", currentLatitude, currentLongitude);
}

bool postToSupabase(String threat, double lat, double lng, String status, int confidence, String threatType) {
  if (!client.connect(SUPABASE_URL.c_str(), 443)) return false;
  StaticJsonDocument<512> doc;
  doc["person_name"] = PERSON_NAME;
  doc["place_name"] = PLACE_NAME;
  doc["phone_number"] = PHONE_NUMBER;
  doc["threat_call"] = threat;
  doc["status"] = "emergency";
  JsonObject gps_location = doc.createNestedObject("gps_location");
  gps_location["lat"] = lat;
  gps_location["lng"] = lng;
  String json;
  serializeJson(doc, json);
  String httpRequest = "";
  httpRequest += "POST /rest/v1/emergency_alerts HTTP/1.1\r\n";
  httpRequest += "Host: " + SUPABASE_URL + "\r\n";
  httpRequest += "Content-Type: application/json\r\n";
  httpRequest += "apikey: " + SUPABASE_ANON_KEY + "\r\n";
  httpRequest += "Authorization: Bearer " + SUPABASE_ANON_KEY + "\r\n";
  httpRequest += "Prefer: return=minimal\r\n";
  httpRequest += "Content-Length: " + String(json.length()) + "\r\n";
  httpRequest += "Connection: close\r\n\r\n";
  httpRequest += json;
  client.print(httpRequest);
  unsigned long timeout = millis();
  String response = "";
  bool success = false;
  while (client.connected() && millis() - timeout < 10000) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      response += line + "\n";
      if (line.startsWith("HTTP/") && (line.indexOf("200") > 0 || line.indexOf("201") > 0)) success = true;
    }
  }
  client.stop();
  return success;
}
