#include <Wire.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "i2c_helpers.h"
#include "wifi_FS_helpers.h"
#include "ESPAsyncWebServer.h"

// Accel Registers Addresses
#define MMA_MODE_REG 0x07
#define MMA_OUTX_REG 0x00  // SPO2 REG
#define MMA_OUTY_REG 0x01  // BPM REG

// Temp Register
#define TEMP_REG 0x00


// I2C SESNORS ADDRESSES
#define ACCEL_ADDRESS 0x4C
#define TEMP_ADDRESS 0x48

// Set HTTP user/pwd
const char* http_username = "doctor";
const char* http_password = "pwd";

// Measuring temperature
uint8_t temp_lowbyte;
uint8_t temp_highbyte;
int16_t temp_raw;


struct SensorData {
  String macAddress;
  int spo2;
  int bpm;
  float temp;
  // unsigned long time;
};


// // Define maximum number of patients and data entries
// #define MAX_PATIENTS 5
// #define MAX_DATA_SIZE 200

// // Create an array to store patient data based on ESP32's MAC address
// SensorData patientData[MAX_PATIENTS][MAX_DATA_SIZE];
// int patientCounters[MAX_PATIENTS];


AsyncWebServer server(80);
TwoWire ESP32Wemo_I2C = TwoWire(0);


// Get the ESP32 MAC address elli bch ykoun l patient ID
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
  temp_highbyte = readI2cReg(TEMP_REG, TEMP_ADDRESS);
  temp_lowbyte = readI2cReg(TEMP_REG + 1, TEMP_ADDRESS);
  temp_raw = temp_lowbyte | (temp_highbyte << 8);
  temp_raw = (int16_t)(temp_raw) >> 5;
  return temp_raw * 0.125;
}


// Adding a new patient usign esp32 mac add
void addNewPatient() {
  // Find the first empty slot for a new patient
  // int patientIndex = -1;
  // for (int i = 0; i < MAX_PATIENTS; i++) {
  //   if (patientCounters[i] == 0) {
  //     patientIndex = i;
  //     break;
  //   }
  // }

  // // If there is no space for a new patient
  // if (patientIndex == -1) {
  //   Serial.println("No space available for new patient.");
  //   return;
  // }

  // // Initialize the patient data structure for the new patient
  // String patientName = "Patient_" + String(patientIndex);  // Placeholder for the patient's name
  // Serial.println("Adding new patient: " + patientName);

  // // Reset patient data to zero (this ensures no leftover data)
  // for (int i = 0; i < MAX_DATA_SIZE; i++) {
  //   patientData[patientIndex][i] = 0;  // Clear the data
  // }
  
  // // Mark the patient slot as filled by setting patientCounters to a non-zero value
  // patientCounters[patientIndex] = 1;  // 1 means data exists in this slot


  // //http post request 

  // Serial.println("New patient added at index: " + String(patientIndex));


}



// returns new sensor data of a patient
SensorData getVitalData(String esp32ID) {
  // int patientIndex = -1;

  // // Find the patient by matching the ESP32 MAC address
  // for (int i = 0; i < MAX_PATIENTS; i++) {
  //   if (esp32ID == getESP32MACAddress()) {  // Replace with actual MAC address matching logic
  //     patientIndex = i;
  //     break;
  //   }
  // }

  // if (patientIndex == -1) {
  //   Serial.println("Patient not found.");
  //   return nullptr;  // Return null pointer if patient is not found
  // }

  SensorData data;

  // Read raw x and y data from the sensor
  int x = (readI2cReg(MMA_OUTX_REG, ACCEL_ADDRESS) << 2);
  int y = (readI2cReg(MMA_OUTY_REG, ACCEL_ADDRESS) << 2);
  float bodyTemp = getBodyTemperature();

  data.spo2 = map(x, 0, 1023, 70, 100);  // Map SpO2 range
  data.bpm = map(y, 0, 1023, 40, 100);   // Map BPM range
  data.temp = bodyTemp;
  data.macAddress = esp32ID;
  // data.time = millis();


  // // Store the data in the patient's array
  // if (patientCounters[patientIndex] < MAX_DATA_SIZE) {
  //   patientData[patientIndex][patientCounters[patientIndex]++] = data;
  // } else {
  //   // Shift all existing values to the left
  //   for (int i = 1; i < MAX_DATA_SIZE; i++) {
  //     patientData[patientIndex][i - 1] = patientData[patientIndex][i];
  //   }
  //   // Place the new value at the last position
  //   patientData[patientIndex][MAX_DATA_SIZE - 1] = data;
  // }

  // // Return the array of the patient's data
  // return patientData[patientIndex];
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


// to try sinon to comment
// void sendSensorDataAsJSON(SensorData* data) {
//   // Create a JSON document
//   DynamicJsonDocument doc(512);

//   // Populate JSON with SensorData fields
//   doc["time"] = data->time;
//   doc["spo2"] = data->spo2;
//   doc["bpm"] = data->bpm;
//   doc["temp"] = data->temp;

//   // Serialize JSON into a string
//   String jsonString;
//   serializeJson(doc, jsonString);

//   // Debug output
//   Serial.println("Sending JSON: ");
//   Serial.println(jsonString);

//   // HTTP POST request
//   HTTPClient http;
//   http.begin("http://" + WiFi.localIP().toString() + "/patient");
//   http.addHeader("Content-Type", "application/json");

//   // Send POST request with the JSON string
//   int httpResponseCode = http.POST(jsonString);

//   // Handle HTTP response
//   if (httpResponseCode > 0) {
//     String response = http.getString();  // Get the response body
//     Serial.print("HTTP Response Code: ");
//     Serial.println(httpResponseCode);
//     Serial.print("Response: ");
//     Serial.println(response);  // Print the server response
//   } else {
//     Serial.print("Error on sending POST: ");
//     Serial.println(http.errorToString(httpResponseCode).c_str());
//   }

//   http.end();  // Close the connection
// }



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

}

void loop() {
  String esp32ID = getESP32MACAddress();
  SensorData data = getVitalData(esp32ID);

  // // Get the number of elements in the array
  // int patientDataSize = sizeof(patientDataArray) / sizeof(patientDataArray[0]);

  // // Get the last valid SensorData
  // if (patientDataSize > 0) {
  //   SensorData lastData = patientDataArray[patientDataSize - 1];
  //   String condition = evaluatePatientCondition(lastData);
  // }

  delay(5000);
}
