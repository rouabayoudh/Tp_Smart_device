#include <Wire.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "i2c_helpers.h"
#include "wifi_FS_helpers.h"
#include "ESPAsyncWebServer.h"

// I2C SESNORS ADDRESSES
#define ACCEL_ADDRESS 0x4C
#define TEMP_ADDRESS 0x48

// Sensors Registers :

// Accel Registers Addresses
#define MMA_MODE_REG 0x07
#define MMA_OUTX_REG 0x00  // SPO2 REG
#define MMA_OUTY_REG 0x01  // BPM REG

// Temp Register
#define TEMP_REG 0x00


// Set HTTP user/pwd
const char* http_username = "doctor";
const char* http_password = "pwd";

String api_url = "https://nodejs-serverless-patient-monitoring.vercel.app/api";

struct SensorData {
  String macAddress;
  int spo2;
  int bpm;
  float temp;
  // unsigned long time;
  String condition;
};


HTTPClient http;
AsyncWebServer server(80);
TwoWire ESP32Wemo_I2C = TwoWire(0);



String getESP32MACAddress() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String macAddress = "";
  for (int i = 0; i < 6; i++) {
    macAddress += String(mac[i], HEX);
    if (i < 5) macAddress += ":";
  }
  return macAddress;
}


float getBodyTemperature() {
  uint8_t temp_highbyte = readI2cReg(TEMP_REG, TEMP_ADDRESS);
  uint8_t temp_lowbyte = readI2cReg(TEMP_REG + 1, TEMP_ADDRESS);

  uint16_t temp_raw = temp_lowbyte | (temp_highbyte << 8);
  int16_t temp_signed = (int16_t)(temp_raw) >> 5;

  float temperature = temp_signed * 0.125;
  return temperature;
}


// a SensorData struct
SensorData getVitalData(String esp32ID) {

  SensorData data;

  // Read raw x and y data from the sensor
  int x = (readI2cReg(MMA_OUTX_REG, ACCEL_ADDRESS) << 2);
  int y = (readI2cReg(MMA_OUTY_REG, ACCEL_ADDRESS) << 2);
  float bodyTemp = getBodyTemperature();

  data.spo2 = map(x, 0, 1023, 70, 100);  // Map SpO2 range
  data.bpm = map(y, 0, 1023, 40, 100);   // Map BPM range

  data.temp = map(bodyTemp, 0, 100, 35, 40);
  data.macAddress = esp32ID;
  // data.time = millis();
  data.condition = "";

  return data;
}


// returns condition
String evaluatePatientCondition(SensorData data) {
  String condition = "Healthy";
  bool isCritical = false;

  // Initialize messages
  String spo2Message = "";
  String bpmMessage = "";
  String tempMessage = "";

  // Check SpO2 levels
  if (data.spo2 < 90) {
    spo2Message = "Critical: Low Oxygen Level (Seek Immediate Help)";
    isCritical = true;
  } else if (data.spo2 < 95) {
    spo2Message = "Warning: Low Oxygen Level (Consider Monitoring)";
  }

  // Check BPM (Heart Rate)
  if (data.bpm < 40) {
    bpmMessage = "Critical: Very Low Heart Rate (Seek Immediate Help)";
    isCritical = true;
  } else if (data.bpm > 100) {
    bpmMessage = "Warning: High Heart Rate (Monitor Carefully)";
  }

  // Check Body Temperature
  if (data.temp < 36.0) {
    tempMessage = "Warning: Low Body Temperature (Monitor Carefully)";
  } else if (data.temp > 38.0) {
    tempMessage = "Warning: High Body Temperature (Possible Fever)";
  }

  // Overall Critical Condition
  if (data.spo2 < 90 || data.bpm < 40 || data.bpm > 120 || data.temp < 36.4 || data.temp > 39.0) {
    condition = "Critical: Immediate Medical Help Needed!";
  } else if (isCritical) {
    condition = "Critical: Multiple Issues Detected!";
  } else {
    // Combine individual warnings if not critical
    if (spo2Message != "" || bpmMessage != "" || tempMessage != "") {
      condition = "Warnings: \n";
      if (spo2Message != "") condition += spo2Message + "\n";
      if (bpmMessage != "") condition += bpmMessage + "\n";
      if (tempMessage != "") condition += tempMessage + "\n";
    }
  }

  return condition;
}


// Adding a new patient usign esp32 mac add
void addNewPatient(String esp32ID) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi not connected!");
    return;
  }

  DynamicJsonDocument doc(1024);  // Allocates 1024 bytes on the heap.

  doc["mac_address"] = esp32ID;
  doc["name"] = "Patient_" + esp32ID;

  // Serialize JSON object to a string
  String payload;
  serializeJson(doc, payload);

  String url = api_url + "/newPatient";
  Serial.println("URL: " + url);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // Send HTTP POST request with JSON payload
  int httpResponseCode = http.POST(payload);

  int retries = 0;
  while (httpResponseCode <= 0 && retries < 5) {  // Retry up to 5 times
    Serial.println("Retrying POST...");
    httpResponseCode = http.POST(payload);
    retries++;
    delay(1000);  // Wait 1 second between retries
  }

  if (httpResponseCode > 0) {
    Serial.println("HTTP Response code: " + String(httpResponseCode));
    Serial.println("Response: " + http.getString());
  } else {
    Serial.println("Error on sending POST: " + String(http.errorToString(httpResponseCode)));
  }

  http.end();
}



void updatePatientName(String newName, String esp32ID) {
  http.begin(api_url + "/updatePatientName");

  http.addHeader("Content-Type", "application/json");

  // Create a JSON object for the payload
  DynamicJsonDocument doc(1024);
  doc["mac_address"] = esp32ID;
  doc["name"] = newName;

  // Serialize JSON to string
  String payload;
  serializeJson(doc, payload);

  // Send the PUT request with the JSON payload
  int httpResponseCode = http.PUT(payload);

  // Handle the response
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Response: " + response);
  } else {
    Serial.println("Error in HTTP request: " + String(httpResponseCode));
  }

  http.end();
}


void addNewSensorData(String esp32ID, SensorData data) {
  data.condition = evaluatePatientCondition(data);

  http.begin(api_url + "/newSensorData?mac_address=" + esp32ID);
  // Set content type and authentication headers if needed
  http.addHeader("Content-Type", "application/json");

  // Create a JSON object for the payload
  DynamicJsonDocument doc(1024);
  doc["spo2"] = data.spo2;
  doc["bpm"] = data.bpm;
  doc["body_temp"] = data.temp;
  doc["condition"] = data.condition;

  // Serialize JSON to string
  String payload;
  serializeJson(doc, payload);

  // Send the POST request with the JSON payload
  int httpResponseCode = http.POST(payload);

  // Handle the response
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Response: " + response);
  } else {
    Serial.println("Error in HTTP request: " + String(httpResponseCode));
  }

  http.end();
}


void setupWebServer() {
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->authenticate(http_username, http_password) == false)
      return request->requestAuthentication();
    else
      request->send(LittleFS, "/index.html", "text/html", false);
  });

  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/styles.css", "text/css");
  });

  server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/app.js", "application/javascript");
  });

  server.begin();
}


void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println("App Launched");

  ESP32Wemo_I2C.begin(SDA_Pin, SCL_Pin, 400000);

  // Enable the accel
  writeI2cReg(MMA_MODE_REG, 0x01, ACCEL_ADDRESS);
  Serial.println("Accel ENABLED");

  initFileSystem();
  connectToWifi();

  setupWebServer();
  Serial.println("Web server available at: http://" + WiFi.localIP().toString());

  String esp32ID = getESP32MACAddress();
  Serial.println(esp32ID);


  addNewPatient(esp32ID);
  // updatePatientName("Amélie Amélie", esp32ID);
}

void loop() {
  String esp32ID = getESP32MACAddress();
  SensorData data = getVitalData(esp32ID);
  addNewSensorData(esp32ID, data);

  delay(5000);
}
