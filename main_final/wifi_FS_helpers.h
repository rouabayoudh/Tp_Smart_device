#ifndef WEBSERVER_HELPERS_H
#define WEBSERVER_HELPERS_H

#include <WiFi.h>
#include "LittleFS.h"

// Replace with your network credentials
const char* ssid = "dou";
const char* password = "dou3aa2.";



// Function to connect to Wi-Fi
void connectToWifi(void) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
}

// Function to initialize LittleFS (file system)
void initFileSystem() {
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  Serial.println("File system mounted successfully.");
}


#endif
