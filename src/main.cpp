#include <WiFi.h>
//#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <time.h>
#include <Secrets.h>
#include <Inverter.h>
#include <Setups.h>
#include <ArduinoJson.h>
#include <MQTT.h>
#include <Settings.h>
#include <TimerService.h>
#include <Arduino.h>
#include <CRCService.h>

String readSerial2Response(String cmd) {
  String result = "";
  unsigned long timeout = millis() + 2000;  // 2000 milliseconds timeout
  while (millis() < timeout) {
    while (Serial2.available()) {
      char c = Serial2.read();
      result += c;
    }
    if (result.endsWith("\r")) break;
  }
  
  if (result.length() < 4) {
    Serial.println(printLocalTime() + " ❌ Response too short or no response received.");
    return "";
  }

  if (isValidStringMessageWithCRC(&result)) {
    Serial.println(printLocalTime() + " ✅ CRC is valid.");
  } else {
    Serial.println(printLocalTime() + " ❌ CRC is INVALID.");
    return "";
  }
  
  if (result == "(NAK" || result == "") {
    Serial.println(printLocalTime() + " ❌ No response or NAK received.");
    return "";
  }

  result.trim();                       // Removes leading/trailing whitespace, including \n, \r
  result.remove(result.length() - 2);  // Remove last 2 characters, it is 2 byte CRC XMODEM

  mqtt_data(printLocalTime() + "=> Send: [" + cmd + "], Received: [" + result + "].");

  return result;
}

// String sendCommand(String cmd) {
//   Serial2.write(cmd.c_str());
//   cmd.remove(cmd.length() - 3);  // Remove last 3 characters
//   return readSerial2Response(cmd);
// }

String sendCommand(String cmd) {
  // Copy original command before adding CRC
  String originalCmd = cmd;

  // Calculate CRC
  uint8_t buffer[64];
  size_t len = cmd.length();
  if (len > sizeof(buffer)) len = sizeof(buffer);  // Safety check

  cmd.getBytes(buffer, len + 1);  // +1 ensures null terminator for safety
  uint16_t crc = crcXmodem(buffer, len);

  // Append CRC and \r to the command
  cmd += (char)((crc >> 8) & 0xFF);  // CRC MSB
  cmd += (char)(crc & 0xFF);         // CRC LSB
  cmd += '\r';                       // End with carriage return

  // Send full binary-safe command
  Serial2.write(cmd.c_str(), cmd.length());

  // Read response using original (non-CRC) command for context
  return readSerial2Response(originalCmd);
}

void ProcessTopic(String topic, String msg) {
  if (topic == "inverter/cmd" & msg == "true") {
    sendCommand("POP00");  // Set output source priority to utility first
  } else if (topic == "inverter/cmd" & msg == "false") {
    sendCommand("POP01");  // Set output source priority to solar first
  } 
  else if (msg == "Restart") {
      // --- The reboot command ---
      ESP.restart();
  }
  else {
      sendCommand(msg);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT message for topic:[" + String(topic) + "], received: [");
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.println(msg + "].");
  ProcessTopic(String(topic), msg);
  // Add command handling if needed
}

int readField(String response, int fieldIndex) {
  int currentIndex = 0;
  int lastSpace = -1;
  for (int i = 0; i < response.length(); i++) {
    if (response.charAt(i) == ' ') {
      currentIndex++;
      if (currentIndex == fieldIndex) {
        int nextSpace = response.indexOf(' ', i + 1);
        if (nextSpace == -1) nextSpace = response.length();
        String field = response.substring(i + 1, nextSpace);
        Serial.print("field: [");
        Serial.print(field.toInt());
        Serial.print("],fieldIndex: [");
        Serial.print(fieldIndex);
        Serial.println("].");

        return field.toInt();
      }
    }
  }
  return -1;
}

void sendAndReceive() {
  //String response = sendCommand(QPIGS);
  String response = sendCommand("QPIGS");

  if(response == "(NAK" || response == "") {
    Serial.println(printLocalTime() + " ❌ No response or NAK received.");
    return;
  }

  int batPercent = readField(response, 10);
  int apparentPowerUsing = readField(response, 4);
  int apparentPowerPV = readField(response, 19);
  int PVInputCurrent = readField(response, 21);

  //String responseQmod = sendCommand(QMOD);
  String responseQmod = sendCommand("QMOD");

  if (batPercent <= 55 & batPercent > 1) {
    if (responseQmod == "(B") {
      mqtt_log("BatPercent is: <= [" + String(batPercent) + "%] and Mode is: Solar/Battery, set it to Utility.");
      //Set output source priority,
      // 00 for utility first,
      //sendCommand(POP00);
      sendCommand("POP00");
    } //else {
    //   mqtt_log("BatPercent is: <= 55 and ResponseQmod is: " + responseQmod + ", so do nothing.");
    // }
  } else if (batPercent == 100 & response == "(L" & isDaytime == true) {
    //Set output source priority,
    // 01 for solar first,
    mqtt_log("BatPercent is: = 100 and Mode is: Utility, isDaytime = true.");
    sendCommand("POP01");
    //sendCommand("POP01" + crc.calculate("POP01") + '\n');
  }
  else if(apparentPowerUsing > 5000)
  {
      // 00 for utility first,
      sendCommand("POP00");
  }
  //  else {
  //   sendCommand(POP01);
  //   readSerial2Response();  // Do not care about result.
  // }

  Serial.print("batPercent: [" + String(batPercent) + "], ");
  Serial.print("apparentPowerUsing: [" + String(apparentPowerUsing) + "], ");
  Serial.print("apparentPowerPV: [" + String(apparentPowerPV) + "], ");
  Serial.println("responseQmod: [" + responseQmod + "].");
}

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  Serial.println("Booting up and setting up WIFI.");

  Serial2.begin(2400, SERIAL_8N1, RXD2, TXD2);  // RX=16, TX=17 (modify if needed)
  Serial2.setRxBufferSize(SERIAL_SIZE_RX);

  SetupWifi();
  //setupOTA();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  // Perform initial NTP sync
  syncNtpTime();

  // Get initial time for current day to decide if we need to call API
  struct tm initialTime;
  getLocalTime(&initialTime);
  lastApiCallDay = initialTime.tm_mday;  // Set current day as last API call day
  lastCheckedHour_daytimeFlag = initialTime.tm_hour;

  // Fetch sunrise/sunset from API immediately after initial NTP sync
  fetchSunriseSunsetFromAPI();

  // Set the initial daytime flag
  updateDaytimeFlag();

  mqtt_log("✅ Setup complete.");
}

void loop() {
  //ArduinoOTA.handle();

  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Get time from local RTC (does NOT contact NTP server)
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    mqtt_log("Failed to get local time in loop.");
    delay(1000);  // Wait a bit before retrying
    return;
  }

  // --- Hourly NTP re-sync exactly on the hour ---
  if (WiFi.status() == WL_CONNECTED && timeinfo.tm_min == 0 && timeinfo.tm_sec == 0 && timeinfo.tm_hour != lastCheckedHour_daytimeFlag) {
    Serial.println("On the dot: Hourly NTP re-sync due...");
    syncNtpTime();
    // After re-sync, immediately update the daytime flag
    updateDaytimeFlag();
    // Reset the lastCheckedHour_daytimeFlag to current hour
    lastCheckedHour_daytimeFlag = timeinfo.tm_hour;
    lastNtpSync = millis(); // Update the last sync time
  }

  // --- Daily API Call for Sunrise/Sunset ---
  // Call API at the start of a new day, or if the current day has changed
  if (timeinfo.tm_mday != lastApiCallDay) {
    Serial.println("New day detected! Fetching new sunrise/sunset times from API...");
    fetchSunriseSunsetFromAPI();
    // After fetching, immediately update the daytime flag
    updateDaytimeFlag();
    // Reset the lastCheckedHour_daytimeFlag to trigger on the next actual hour change
    lastCheckedHour_daytimeFlag = timeinfo.tm_hour;
    lastApiCallDay = timeinfo.tm_mday;
  }

    // --- every 10-Second sendAndReceive Timer ---
  unsigned long currentMillis = millis();
  if (currentMillis - lastSerialSendTime >= serialSendInterval) {
    lastSerialSendTime = currentMillis;  // Update the last time this ran
    sendAndReceive();                    // Call your periodic function
  }
}