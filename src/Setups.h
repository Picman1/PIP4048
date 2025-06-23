#include <Settings.h>

void SetupWifi() {
  WiFi.config(local_IP, gateway, subnet, primaryDNS);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  Serial.print("Signal strength: [");
  Serial.print(WiFi.RSSI());
  Serial.println("] dBm");
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