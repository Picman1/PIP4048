#ifndef TIMERSERVICE_H
#define TIMERSERVICE_H

#include <MQTT.h>
#include <HTTPClient.h> // For HTTP requests
#include <Settings.h>

// Function to print the local time
String printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time (RTC not set or invalid).");
    return "";
  }

  char timeStr[64];
  // Format: YYYY-MM-DD HH:MM:SS (DayOfWeek)
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S (%A)", &timeinfo);
  //Serial.print("Current Local Time: ");
  //Serial.println(timeStr);
  return timeStr;//formatTimeToString(timeinfo);
}

/**
 * @brief Converts a struct tm object into a formatted String.
 * * @param timeinfo A constant reference to the struct tm object to format.
 * @return A String containing the formatted date and time (e.g., "YYYY-MM-DD HH:MM:SS").
 */
String formatTimeToString(const struct tm& timeinfo) {
  // Create a character buffer to hold the formatted string.
  // Make sure this buffer is large enough for your desired format!
  // A common format like "YYYY-MM-DD HH:MM:SS" is about 19 characters + null terminator.
  // Add space for day names, month names, etc., if your format changes.
  char timeBuffer[30]; // 30 bytes should be sufficient for many common formats.

  // Use strftime to format the timeinfo into the buffer.
  // "%Y-%m-%d %H:%M:%S" is a common format (YYYY-MM-DD HH:MM:SS)
  size_t charsWritten = strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

  // Optional: Check if the buffer was large enough
  if (charsWritten == 0) {
    Serial.println("Error: Time buffer too small or formatting failed!");
    return "ERROR_FORMATTING_TIME"; // Return an error string
  }

  // Convert the C-style string to an Arduino String and return it.
  return String(timeBuffer);
}

// // --- Function to perform NTP synchronization ---
// void syncNtpTime() {
//   Serial.println("Attempting NTP time synchronization...");
//   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

//   struct tm timeinfo;
//   // Wait up to 5 seconds for NTP sync
//   if (!getLocalTime(&timeinfo, 5000)) {
//     Serial.println("Failed to obtain time from NTP server after sync attempt.");
//     mqtt_log("Failed to obtain time from NTP server after sync attempt.");
//     // In a real application, you might want to try again or go into a low-power mode
//   } else {
//     Serial.print("NTP synchronized. Current time: ");
//     Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

//     // Convert the C-style string to an Arduino String
//     mqtt_log("NTP synchronized. Current time: " + formatTimeToString(timeinfo));
//     lastNtpSyncTime = time(NULL); // Update the last sync time
//   }
// }

// Function to synchronize time with NTP server
void syncNtpTime() {
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);
  // For SAST (UTC+2) and no daylight saving: gmtOffset_sec = 2 * 3600 = 7200, daylightOffset_sec = 0
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2); // Use ntpServer1 and ntpServer2

  // Set the Timezone environment variable for correct time interpretation
  setenv("TZ", TZ_INFO, 1);
  tzset(); // Apply the timezone

  Serial.print("Waiting for NTP time...");
  time_t now;
  struct tm timeinfo;
  int attempts = 0;
  const int MAX_NTP_ATTEMPTS = 30; // Max attempts to get time (e.g., 30 * 1000ms = 30 seconds)

  while (attempts < MAX_NTP_ATTEMPTS) {
    time(&now); // Get current time as time_t (seconds since epoch)
    localtime_r(&now, &timeinfo); // Convert time_t to tm struct for breakdown

    // Check if the year is valid (NTP usually returns a year after 2000)
    if (timeinfo.tm_year > (2000 - 1900)) { // tm_year is years since 1900
      mqtt_log(printLocalTime() + "Time synchronized successfully: [" + formatTimeToString(timeinfo) + "].");
      //printLocalTime();
      return; // Exit function if time is synchronized
    }
    Serial.print(".");
    delay(1000); // Wait 1 second before retrying
    attempts++;
  }

  mqtt_log(printLocalTime() + "Failed to get time from NTP server after multiple attempts.");
}

// --- REVISED Helper to convert ISO8601 time string to local time_t (WITHOUT timegm) ---
time_t parseISO8601ToUnix(const char* isoTimeStr) {
    struct tm tm_utc; // Use tm_utc for clarity that this holds UTC parsed values
    if (sscanf(isoTimeStr, "%d-%d-%dT%d:%d:%d",
               &tm_utc.tm_year, &tm_utc.tm_mon, &tm_utc.tm_mday,
               &tm_utc.tm_hour, &tm_utc.tm_min, &tm_utc.tm_sec) == 6) {

        tm_utc.tm_year -= 1900; // tm_year is years since 1900
        tm_utc.tm_mon -= 1;     // tm_mon is 0-11 (Jan=0)
        tm_utc.tm_isdst = 0;    // Explicitly set to 0 for UTC time (no DST for UTC)

        // Store current timezone settings to restore them later
        char* old_tz = getenv("TZ");
        char old_tz_buffer[64]; // Sufficient buffer for typical TZ strings
        if (old_tz) {
            strncpy(old_tz_buffer, old_tz, sizeof(old_tz_buffer) - 1);
            old_tz_buffer[sizeof(old_tz_buffer) - 1] = '\0';
        } else {
            old_tz_buffer[0] = '\0'; // No old TZ to restore
        }

        // Temporarily set timezone to UTC for mktime to interpret tm_utc correctly
        setenv("TZ", "UTC", 1); // Set TZ environment variable to "UTC", overwrite if exists
        tzset(); // Re-initialize time conversion information based on new TZ

        // Convert the parsed UTC tm struct to a UTC Unix timestamp using mktime
        time_t utc_timestamp_from_api = mktime(&tm_utc);

        // Restore original timezone settings
        if (old_tz_buffer[0] != '\0') {
            setenv("TZ", old_tz_buffer, 1);
        } else {
            unsetenv("TZ"); // If there was no original TZ, unset it
        }
        tzset(); // Re-initialize time conversion based on original TZ

        if (utc_timestamp_from_api == (time_t)-1) {
            Serial.println("Error converting parsed UTC time to UTC timestamp.");
            return 0;
        }

        // Add our configured GMT offset and DST offset to convert it to our desired local time_t
        time_t local_time = utc_timestamp_from_api + gmtOffset_sec + daylightOffset_sec;

        return local_time;
    }
    return 0; // Return 0 on parsing error
}

// --- Function to fetch sunrise/sunset from API ---
void fetchSunriseSunsetFromAPI() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Cannot fetch API data.");
    return;
  }

  HTTPClient http;
  String url = "https://api.sunrise-sunset.org/json?";
  url += "lat=";
  url += String(PRETORIA_LATITUDE, 6); // 6 decimal places for precision
  url += "&lng=";
  url += String(PRETORIA_LONGITUDE, 6);
  url += "&formatted=0"; // Request unformatted UTC times for easier parsing

  // Get current date for the API request (to ensure we get today's sunrise/sunset)
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to get local time for API date parameter.");
      return;
  }
  char dateStr[11]; // YYYY-MM-DD + null terminator
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
  url += "&date=";
  url += dateStr;

  Serial.print("Fetching sunrise/sunset from: ");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.print("API Response: ");
    Serial.println(payload);

    // DynamicJsonDocument is preferred for flexible memory allocation
    const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(11) + 500; // Adjust capacity as needed
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      sunriseTimeToday = 0;
      sunsetTimeToday = 0;
    } else {
      if (doc["status"] == "OK") {
        const char* sunrise_str = doc["results"]["sunrise"];
        const char* sunset_str = doc["results"]["sunset"];

        sunriseTimeToday = parseISO8601ToUnix(sunrise_str);
        sunsetTimeToday = parseISO8601ToUnix(sunset_str);

        mqtt_log(printLocalTime() + "Parsed Sunrise (local time): " + String(ctime(&sunriseTimeToday)) + ": Parsed Sunset (local time): " + String(ctime(&sunsetTimeToday)));

        lastApiCallTime = time(NULL); // Mark successful API call time
        lastApiCallDay = timeinfo.tm_mday; // Mark the day of the successful API call
      } else {
        Serial.print("API returned an error status: ");
        Serial.println(doc["status"].as<String>());
        mqtt_log(printLocalTime() + "API returned an error status: " + doc["status"].as<String>());
        sunriseTimeToday = 0;
        sunsetTimeToday = 0;
      }
    }
  } else {
    Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    sunriseTimeToday = 0; // Clear times on error
    sunsetTimeToday = 0;
  }
  http.end(); // Free resources
}

// --- Function to update the isDaytime flag ---
void updateDaytimeFlag() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    mqtt_log(printLocalTime() + "Failed to get local time for flag update.");
    return;
  }

  // Get current time as Unix timestamp for comparison
  time_t currentTime = time(NULL);

  Serial.print("Current local time for flag check: ");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.printf("Sunrise (local): %s", ctime(&sunriseTimeToday));
  Serial.printf("Sunset (local): %s", ctime(&sunsetTimeToday));

  // Check if we have valid sunrise/sunset times
  if (sunriseTimeToday != 0 && sunsetTimeToday != 0) {
    if (currentTime >= sunriseTimeToday && currentTime < sunsetTimeToday) {
      isDaytime = true;
      //Serial.println("It is currently DAYTIME (based on API data).");
      mqtt_log(printLocalTime() + "It is currently DAYTIME (based on API data).");
    } else {
      isDaytime = false;
      //Serial.println("It is currently NIGHTTIME (based on API data).");
      mqtt_log(printLocalTime() + "It is currently NIGHTTIME (based on API data).");
    }
  } else {
    // Fallback if API data is missing or invalid
    Serial.println("API sunrise/sunset data missing or invalid. Falling back to simple hour-based check.");
    // APPROXIMATE Day/Night Detection (Fallback)
    const int FALLBACK_DAYTIME_START_HOUR = 6;
    const int FALLBACK_DAYTIME_END_HOUR = 18;
    if (timeinfo.tm_hour >= FALLBACK_DAYTIME_START_HOUR && timeinfo.tm_hour < FALLBACK_DAYTIME_END_HOUR) {
      isDaytime = true;
      Serial.printf("It is currently DAYTIME (fallback: between %d:00 and %d:00).\n", FALLBACK_DAYTIME_START_HOUR, FALLBACK_DAYTIME_END_HOUR);
      mqtt_log(printLocalTime() + "It is currently DAYTIME (fallback: between " + String(FALLBACK_DAYTIME_START_HOUR) + " and " + String(FALLBACK_DAYTIME_END_HOUR) + ").");
    } else {
      isDaytime = false;
      Serial.printf("It is currently NIGHTTIME (fallback: outside %d:00 - %d:00).\n", FALLBACK_DAYTIME_START_HOUR, FALLBACK_DAYTIME_END_HOUR);
      mqtt_log(printLocalTime() + "It is currently NIGHTTIME (fallback: between " + String(FALLBACK_DAYTIME_START_HOUR) + " and " + String(FALLBACK_DAYTIME_END_HOUR) + ").");
    }
  }
}

#endif // TIMERSERVICE_H