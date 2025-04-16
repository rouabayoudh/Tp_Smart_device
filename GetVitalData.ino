#include <Wire.h>

// I2C ADDRESS
#define SENSOR_ADDRESS   0x4C

// Sensor Registers Addresses
#define MMA_MODE_REG         0x07
#define MMA_OUTX_REG         0x00 // SPO2 REG
#define MMA_OUTY_REG         0x01 // BPM REG

// I2C0 PINS
#define SDA_Pin 21
#define SCL_Pin 22

TwoWire ESP32Wemo_I2C = TwoWire(0);

// Define a struct to hold SpO2, BPM, and timestamp
struct SensorData {
    int spo2;
    int bpm;
    unsigned long time;
};

// Array to store all data
const int size = 1000; // Adjust the size as needed
SensorData VitalInfo[size];
int index = 0; // Tracks the current number of stored records

void writeI2cReg(uint8_t RegAddr, uint8_t Value) {
    ESP32Wemo_I2C.beginTransmission(SENSOR_ADDRESS);
    ESP32Wemo_I2C.write(RegAddr);
    ESP32Wemo_I2C.write(Value);
    if (ESP32Wemo_I2C.endTransmission(true) != 0) {
        Serial.println("Problem writing to I2C device");
        exit(0);
    }
}

uint8_t readI2cReg(uint8_t RegAddr) {
    ESP32Wemo_I2C.beginTransmission(SENSOR_ADDRESS);
    ESP32Wemo_I2C.write(RegAddr);
    if (ESP32Wemo_I2C.endTransmission(false)) { // If != 0
        Serial.println("Problem writing without stop");
        exit(0);
    }
    ESP32Wemo_I2C.requestFrom(SENSOR_ADDRESS, 0x01);
    return (ESP32Wemo_I2C.read());
}

// Function to read SpO2 and BPM, store data in the array, and return the latest reading
SensorData getVitalData() {
    if (index >= size) {
        Serial.println("Error: VitalInfo array is full.");
        exit(0);
    }

    SensorData data;

    // Read raw x and y data from the sensor
    int x = (readI2cReg(MMA_OUTX_REG) << 2);
    int y = (readI2cReg(MMA_OUTY_REG) << 2);

    // Map the raw data to the desired ranges
    data.spo2 = map(x, 0, 1023, 70, 100); // Map SpO2 range (Badel range selon les valeurs kan s8ar na9s range w l3aks)
    data.bpm = map(y, 0, 1023, 40, 100);  // Map BPM range (Badel range selon les valeurs kan s8ar na9s range w l3aks)

    // Capture the current time
    data.time = millis();

    // Store the data in the array
    VitalInfo[index++] = data;

    return data;
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("App Launched");

    ESP32Wemo_I2C.begin(SDA_Pin, SCL_Pin, 400000);

    // Enable the sensor
    writeI2cReg(MMA_MODE_REG, 0x01);
    Serial.println("Sensor ENABLED");
}

void loop() {
    // Call getVitalData to read sensor data and store it
    SensorData data = getVitalData();

    // Print the most recent result w 3andek all the data are stored in VitalData array (VitalData[i].spo2 | VitalData[i].bpm | VitalData[i].time)
    Serial.print("Oxygen Saturation (SpO2): ");
    Serial.println(data.spo2);
    Serial.print("Heartbeat (BPM): ");
    Serial.println(data.bpm);
    Serial.print("Timestamp (ms): ");
    Serial.println(data.time);

    delay(1000);
}

