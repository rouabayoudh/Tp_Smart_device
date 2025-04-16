#include <Wire.h>
#include <LittleFS.h>

#include "i2c_helpers.h"
#include "wifi_FS_helpers.h"
#include "ESPAsyncWebServer.h"

// Sensor Registers Addresses
#define MMA_MODE_REG 0x07
#define MMA_OUTX_REG 0x00  // SPO2 REG
#define MMA_OUTY_REG 0x01  // BPM REG

// Set HTTP user/pwd
const char* http_username = "doctor";
const char* http_password = "pwd";

AsyncWebServer server(80);
TwoWire ESP32Wemo_I2C = TwoWire(0);

// Define a struct to hold SpO2, BPM, and timestamp
struct SensorData {
  int spo2;
  int bpm;
  float temp;
  unsigned long time;
};

// Define maximum number of patients and data entries
#define MAX_PATIENTS 5
#define MAX_DATA_SIZE 200

// Create an array to store patient data based on ESP32's MAC address
SensorData patientData[MAX_PATIENTS][MAX_DATA_SIZE];
int patientCounters[MAX_PATIENTS];


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

// Function to read SpO2 and BPM, store data in the array, and return the latest reading
SensorData getVitalData(String esp32ID) {
  int patientIndex = -1;

  // Find the patient by matching l id elli houa l mac add
  for (int i = 0; i < MAX_PATIENTS; i++) {
    if (esp32ID == getESP32MACAddress()) {
      patientIndex = i;
      break;
    }
  }

  if (patientIndex == -1) {
    Serial.println("Patient not found.");
    return SensorData();
  }

  SensorData data;

  // Read raw x and y data from the sensor
  int x = (readI2cReg(MMA_OUTX_REG) << 2);
  int y = (readI2cReg(MMA_OUTY_REG) << 2);

  // Map the raw data to the desired ranges
  data.spo2 = map(x, 0, 1023, 70, 100);  // Map SpO2 range
  data.bpm = map(y, 0, 1023, 40, 100);   // Map BPM range

  data.temp = random(950, 1050) / 10.0;  // random pour le moment

  // Capture the current time
  data.time = millis();

  // Store the data in the patient's array
  if (patientCounters[patientIndex] < MAX_DATA_SIZE) {
    patientData[patientIndex][patientCounters[patientIndex]++] = data;
  } else {
    // Shift all existing values to the left
    for (int i = 1; i < MAX_DATA_SIZE; i++) {
      patientData[patientIndex][i - 1] = patientData[patientIndex][i];
    }
    // Place the new value at the last position
    patientData[patientIndex][MAX_DATA_SIZE - 1] = data;
  }

  return data;
}



String evaluatePatientCondition(SensorData data) {

  String condition = "Healthy";

  // Check SpO2 levels
  if (data.spo2 < 90) {
    condition = "Critical: Low Oxygen Level (Seek Immediate Help)";
  } else if (data.spo2 < 95) {
    condition = "Warning: Low Oxygen Level (Consider Monitoring)";
  }

  // Check BPM (Heart Rate)
  if (data.bpm < 40) {
    condition = "Critical: Very Low Heart Rate (Seek Immediate Help)";
  } else if (data.bpm > 100) {
    condition = "Warning: High Heart Rate (Monitor Carefully)";
  }

  // Check Body Temperature
  if (data.temp < 36) {
    condition = "Warning: Low Body Temperature (Monitor Carefully)";
  } else if (data.temp > 38) {
    condition = "Warning: High Body Temperature (Possible Fever)";
  }

  // Critical Condition Check
  if (data.spo2 < 90 || data.bpm < 40 || data.bpm > 120 || data.temp < 36.4 || data.temp > 39.0) {
    condition = "Critical: Immediate Medical Help Needed!";
  }

  return condition;
}



// Function to set up the web server
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


  server.on("/patient/{id}", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.print("ma dkhalch ll if");
    if (request->args() > 0) {
      String patientId = request->pathArg(0);  // Get the value of ":id"

      // Fetch the patient's data
      SensorData data = getVitalData(patientId);
      String condition = evaluatePatientCondition(data);

      // Build JSON response
      String jsonResponse = "{\"id\": \"" + patientId + "\", \"spo2\": " + String(data.spo2) + ", \"bpm\": " + String(data.bpm) + ", \"condition\": \"" + condition + "\", \"timestamp\": " + String(data.time) + "}";
      Serial.print("dkhal ll if");
      request->send(200, "application/json", jsonResponse);
    } else {
      request->send(400, "application/json", "{\"error\": \"Invalid or missing patient ID\"}");
    }
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
  writeI2cReg(MMA_MODE_REG, 0x01);
  Serial.println("Accel ENABLED");


  // Enable the temp sensor


  initFileSystem();
  connectToWifi();
  setupWebServer();
}

void loop() {
  String esp32ID = getESP32MACAddress();
  SensorData data = getVitalData(esp32ID);
  String condition = evaluatePatientCondition(data);

  // Print the most recent result
  // Serial.print("Oxygen Saturation (SpO2): ");
  // Serial.println(data.spo2);
  // Serial.print("Heartbeat (BPM): ");
  // Serial.println(data.bpm);
  // Serial.print("Timestamp (ms): ");
  // Serial.println(data.time);

  // Serial.print("Condition: ");
  // Serial.println(condition);

  Serial.println("Web server available at: http://" + WiFi.localIP().toString());
  Serial.println(esp32ID);



  // float output = runInference(data.spo2, data.bpm, data.temp);
  // Serial.print("Prediction:");
  // Serial.println(output);

  delay(7000);
}
