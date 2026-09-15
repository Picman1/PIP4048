#include <Settings.h>
#include <ESP32Ping.h>

void WaitForInternet() {
  Serial.print("\nWaiting for a ping reply from MQTT server ");
  Serial.println(mqtt_server);
  
  unsigned long startTime = millis();
  const unsigned long maxWaitTime = 60000; // 60 seconds max wait
  int attempts = 0;
  const int maxAttempts = 60;

  while (attempts < maxAttempts && millis() - startTime < maxWaitTime) {
    attempts++;
    
    Serial.print("   Attempt ");
    Serial.print(attempts);
    Serial.print("/");
    Serial.print(maxAttempts);
    Serial.print(" - ");
    
    // Send one ping to check whether the MQTT server is reachable.
    if (Ping.ping(mqtt_server, 1)) {
      Serial.println("MQTT server replied to ping.");
      return;
    }
    
    Serial.println("No ping reply from MQTT server.");
    
    if (attempts < maxAttempts) {
      delay(1000); // Wait 1 seconds before retry
    }
  }

  Serial.println("MQTT server ping timeout. Continuing anyway...\n");
}

void SetupWifi() {
  WiFi.config(local_IP, gateway, subnet, primaryDNS);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 10000; // 10 seconds

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect. Restarting...");
    ESP.restart(); // Restart the ESP32
  }

  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  Serial.print("Signal strength: [");
  Serial.print(WiFi.RSSI());
  Serial.println("] dBm");

  // Wait for a ping reply from the MQTT server before proceeding.
  WaitForInternet();
}

// void setupOTA() {
//   // OTA Config
//   ArduinoOTA.setPassword(otaPassword);  // Set OTA password
//   ArduinoOTA.setHostname("esp32-inverter");
  
//   // Callbacks for OTA
//   ArduinoOTA.onStart([]() {
//     Serial.println("OTA Start");
//   });

//   ArduinoOTA.onEnd([]() {
//     Serial.println("\nOTA End");
//   });

//   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//     Serial.printf("Progress: %u%%\r", (progress * 100) / total);
//   });

//   ArduinoOTA.onError([](ota_error_t error) {
//     Serial.printf("Error[%u]: ", error);
//     if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
//     else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
//     else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
//     else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
//     else if (error == OTA_END_ERROR) Serial.println("End Failed");
//   });

//   ArduinoOTA.begin();
//   Serial.println("OTA ready");
// }
