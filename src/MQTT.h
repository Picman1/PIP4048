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

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected.");
      client.subscribe(subscribe_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

#endif // MQTT_H