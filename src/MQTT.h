#ifndef MQTT_H
#define MQTT_H

#include <Settings.h>

void mqtt_data(const String& message) {
  if (client.connected()) {
    client.publish(publish_topic_data, message.c_str());
  }

  Serial.println(message);
}

void mqtt_log(const String& message) {
  if (client.connected()) {
    client.publish(publish_topic_log, message.c_str());
  }

  Serial.println(message);
}

// void mqtt_mode(const String& message) {
//   if (client.connected()) {
//     client.publish(publish_topic_mode, message.c_str());
//   }

//   Serial.println(message);
// }

bool tryConnectMQTT() {
  const int maxAttempts = 10;
  for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
    if (client.connected()) {
      return true;
    }

    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected.");
      client.subscribe(subscribe_topic);
      return true;
    }

    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.print(". Attempt ");
    Serial.print(attempt);
    Serial.println(" of 10.");
    if (attempt < maxAttempts) {
      delay(5000);
    }
  }
  return false;
}

void reconnectMQTT() {
  if (tryConnectMQTT()) {
    return;
  }

  Serial.println("MQTT failed 10 times. Reconnecting WiFi...");
  // Keep the existing WiFi credentials and static network configuration.
  WiFi.disconnect();
  delay(500);
  WiFi.begin(ssid, password);

  const unsigned long wifiTimeout = 20000UL;
  const unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < wifiTimeout) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi reconnected. Retrying MQTT...");
    if (tryConnectMQTT()) {
      return;
    }
    Serial.println("MQTT failed another 10 times. Recovery failed.");
  } else {
    Serial.println("WiFi reconnection failed. Recovery failed.");
  }

  Serial.println("Rebooting ESP32 in 2 seconds...");
  delay(2000);
  ESP.restart();
}

#endif // MQTT_H