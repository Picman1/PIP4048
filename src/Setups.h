#include <Settings.h>

void WaitForInternet() {
  Serial.println("\n🌐 Waiting for internet connectivity (testing DNS to 8.8.8.8)...");
  
  unsigned long startTime = millis();
  const unsigned long maxWaitTime = 60000; // 60 seconds max wait
  int attempts = 0;
  const int maxAttempts = 10;

  while (attempts < maxAttempts && millis() - startTime < maxWaitTime) {
    attempts++;
    
    IPAddress resolvedIP;
    Serial.print("   Attempt ");
    Serial.print(attempts);
    Serial.print("/");
    Serial.print(maxAttempts);
    Serial.print(" - ");
    
    // Try DNS lookup to verify internet connectivity
    if (WiFi.hostByName("8.8.8.8", resolvedIP)) {
      Serial.println("✅ Internet verified (DNS resolved 8.8.8.8)");
      return;
    }
    
    Serial.println("❌ DNS lookup failed");
    
    if (attempts < maxAttempts) {
      delay(2000); // Wait 2 seconds before retry
    }
  }

  Serial.println("⚠️  Internet connectivity timeout. Continuing anyway (may experience issues)...\n");
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

  // Wait for actual internet connectivity before proceeding
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