#include <Wire.h>
#include <LittleFS.h>

#include "i2c_helpers.h"
#include "wifi_FS_helpers.h"
#include "model_helpers.h"
#include "ESPAsyncWebServer.h"
#include "model.h"

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
  float tempF;
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

  data.tempF = random(950, 1050) / 10.0; // random pour le moment

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

// Function to process the HTML content and replace placeholders
String processor(const String& var) {
  if (var == "ESP32ID") {
    return getESP32MACAddress();  // Replace with ESP32 MAC address
  }
  return String();
}

float celsiusToFahrenheit(float celsius) {
  return (celsius * 9.0 / 5.0) + 32.0;
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

  // Route to patient /patient/:id
  server.on("/patient/:id", HTTP_GET, [](AsyncWebServerRequest* request) {
    String patientId = request->getParam("id")->value();
    String html = LittleFS.open("/patient.html", "r").readString();    //?????

    // Get the patient's data
    SensorData data = getVitalData(patientId);

    // A CHANGERRRRRRRRRRRRRRRRRRRRRRRRR
    // Replace placeholders in patient.html with real data
    // html.replace("{{patientId}}", patientId);
    // html.replace("{{spO2}}", String(data.spo2));
    // html.replace("{{heartRate}}", String(data.bpm));
    // html.replace("{{timestamp}}", String(data.time));

  // Send the response as JSON including condition, spo2, bpm, and timestamp
  String jsonResponse = "{\"id\": \"" + patientId + "\", \"spo2\": " + String(data.spo2) + 
                        ", \"bpm\": " + String(data.bpm) + ", \"condition\": \"" + condition + 
                        "\", \"timestamp\": " + String(data.time) + "}";
    request->send(200, "text/html", html);
  });

  server.begin();
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("App Launched");

  ESP32Wemo_I2C.begin(SDA_Pin, SCL_Pin, 400000);

  // Enable the sensor
  writeI2cReg(MMA_MODE_REG, 0x01);
  Serial.println("Accel ENABLED");

  initFileSystem();
  connectToWifi();
  setupWebServer();
  setupTFLite();
}

void loop() {
  String esp32ID = getESP32MACAddress();
  SensorData data = getVitalData(esp32ID);

  // Print the most recent result
  Serial.print("Oxygen Saturation (SpO2): ");
  Serial.println(data.spo2);
  Serial.print("Heartbeat (BPM): ");
  Serial.println(data.bpm);
  Serial.print("Timestamp (ms): ");
  Serial.println(data.time);

  float output = runInference(data.spo2, data.bpm, data.tempF);
  Serial.print("Prediction:");
  Serial.println(output);

  delay(1000);
}
