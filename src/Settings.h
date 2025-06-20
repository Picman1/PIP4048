// Settings.h
#ifndef SETTINGS_H
#define SETTINGS_H

#define RXD2 16
#define TXD2 17
#define SERIAL_SIZE_RX 200

const char* mqtt_server = "192.168.1.203";  // RapberryPI IP
const int mqtt_port = 1883;

const char* publish_topic_data = "inverter/data";
const char* publish_topic_log = "inverter/log";
//const char* publish_topic_mode = "inverter/mode";

const char* subscribe_topic = "inverter/cmd";

// --- NTP Server Configuration ---
// You can use multiple NTP servers for redundancy
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";

// --- Timezone Configuration ---
// See https://github.com/nayarsystems/arduino-esp32-sntp/blob/master/src/sntp.cpp for more TZ examples
// South Africa Standard Time (SAST) is UTC+2, no daylight saving
const char* TZ_INFO = "SAST-2SAST,M11.1.0,M3.5.0/2"; // Example for SAST (UTC+2)

const int daylightOffset_sec = 0;

// Timing
// Variables for timed MQTT sending
unsigned long lastSerialSendTime = 0;
const long serialSendInterval = 10000; // 10 seconds

unsigned long lastNtpSync = 0;
const unsigned long ntpSyncInterval = 3600000;  // 1 hour

int lastSyncedHour = -1;

// Static IP configuration
IPAddress local_IP(192, 168, 1, 204);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);

WiFiClient espClient;
PubSubClient client(espClient);

//geographic coordinates for Pretoria, South Africa
const double PRETORIA_LATITUDE = -25.7479;
const double PRETORIA_LONGITUDE = 28.2293;

// SAST is UTC+2
const long  gmtOffset_sec = 2 * 3600;

bool isDaytime = false;

// --- Store Parsed Sunrise/Sunset Times (in local time, seconds since epoch) ---
time_t sunriseTimeToday = 0;
time_t sunsetTimeToday = 0;

// --- Time Tracking for Hourly Update and NTP Re-sync ---
int lastCheckedHour_daytimeFlag = -1; // Stores the hour when the daytime flag was last updated
time_t lastNtpSyncTime = 0;           // Stores the timestamp of the last successful NTP sync
const long NTP_RESYNC_INTERVAL_SECONDS = 3600; // Re-sync NTP every 1 hour (3600 seconds)

// --- Time Tracking for API Call ---
time_t lastApiCallTime = 0;
// We'll call the API once per day (24 hours * 3600 seconds)
const long API_CALL_INTERVAL_SECONDS = 24 * 3600;
// Make sure to call API at the start of a new day to get correct times for that day
int lastApiCallDay = -1;

#endif // SETTINGS_H