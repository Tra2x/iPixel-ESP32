#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#define NTP_SERVER "pool.ntp.org"
#define NTP_SYNC_INTERVAL_MS (24 * 60 * 60 * 1000)  // 24 hours
#define NTP_SYNC_TIMEOUT_MS 10000                    // 10 seconds timeout

class NTPManager {
private:
  unsigned long lastSyncTime = 0;
  bool syncInProgress = false;
  bool timeSet = false;

public:
  NTPManager() {}

  /**
   * Initialize NTP synchronization
   * Should be called once during setup
   */
  void init() {
    Serial.println("[NTP] Initializing NTP Manager...");

    // Configure time for Germany (CET/CEST) with explicit DST rules
    // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer)
    // gmtOffset_sec = 1 hour = 3600 seconds (CET = UTC+1, winter time)
    // daylightOffset_sec = 1 hour = 3600 seconds (CEST = UTC+2, summer time adds 1 more hour)
    configTime(3600, 3600, NTP_SERVER);

    // Set the TZ environment variable with explicit DST transition rules
    // Format: "STD offset DST offset,start,end"
    // CET-1 = Central European Time (UTC+1)
    // CEST-2 = Central European Summer Time (UTC+2)
    // M3.5.0/2 = Last Sunday of March at 2:00 AM (DST starts)
    // M10.5.0/3 = Last Sunday of October at 3:00 AM (DST ends)
    // This ensures DST transitions work even if WiFi is disconnected
    setenv("TZ", "CET-1CEST-2,M3.5.0/2,M10.5.0/3", 1);
    tzset();

    Serial.println("[NTP] NTP Manager initialized (Germany timezone CET/CEST with DST)!");
  }

  /**
   * Synchronize time with NTP server
   * Returns true if sync was successful or already in progress
   */
  bool syncTime() {
    // Don't sync if already syncing
    if (syncInProgress) {
      return true;
    }

    // Check if WiFi is connected
    if (!WiFi.isConnected()) {
      Serial.println("[NTP] WiFi not connected, skipping NTP sync");
      return false;
    }

    // Check if enough time has passed since last sync
    unsigned long now = millis();
    if (timeSet && (now - lastSyncTime) < NTP_SYNC_INTERVAL_MS) {
      return true;  // Not time to sync yet
    }

    syncInProgress = true;
    Serial.println("[NTP] Starting NTP synchronization...");

    // Request time from NTP server with Germany timezone
    // gmtOffset_sec = 3600 (CET = UTC+1)
    // daylightOffset_sec = 3600 (CEST = UTC+2, adds 1 more hour)
    configTime(3600, 3600, NTP_SERVER);

    // Wait for time to be set (with timeout)
    unsigned long startTime = millis();
    time_t now_time = time(nullptr);

    while (now_time < 24 * 3600 && (millis() - startTime) < NTP_SYNC_TIMEOUT_MS) {
      delay(100);
      now_time = time(nullptr);
    }

    syncInProgress = false;

    if (now_time > 24 * 3600) {
      timeSet = true;
      lastSyncTime = millis();

      struct tm timeinfo;
      localtime_r(&now_time, &timeinfo);

      // Display time in German timezone (automatically adjusted by localtime_r)
      Serial.printf("[NTP] Time synchronized (Germany): %04d-%02d-%02d %02d:%02d:%02d (DST: %s)\n",
                    timeinfo.tm_year + 1900,
                    timeinfo.tm_mon + 1,
                    timeinfo.tm_mday,
                    timeinfo.tm_hour,
                    timeinfo.tm_min,
                    timeinfo.tm_sec,
                    timeinfo.tm_isdst > 0 ? "yes" : "no");
      return true;
    } else {
      Serial.println("[NTP] NTP synchronization failed (timeout)");
      return false;
    }
  }

  /**
   * Get current time as struct tm
   * Returns false if time is not yet set
   */
  bool getCurrentTime(struct tm &timeinfo) {
    if (!timeSet) {
      return false;
    }

    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);
    return true;
  }

  /**
   * Get current hour, minute, second
   * Returns false if time is not yet set
   */
  bool getCurrentTimeHMS(int &hour, int &minute, int &second) {
    struct tm timeinfo;
    if (!getCurrentTime(timeinfo)) {
      return false;
    }

    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    second = timeinfo.tm_sec;
    return true;
  }

  /**
   * Get current date information
   * Returns false if time is not yet set
   */
  bool getCurrentDate(int &year, int &month, int &day, int &dayOfWeek) {
    struct tm timeinfo;
    if (!getCurrentTime(timeinfo)) {
      return false;
    }

    year = timeinfo.tm_year % 100;  // Last 2 digits of year
    month = timeinfo.tm_mon + 1;    // 1-12
    day = timeinfo.tm_mday;         // 1-31
    dayOfWeek = timeinfo.tm_wday;   // 0=Sunday, 1=Monday, ..., 6=Saturday
    
    // Convert to 1-7 format (1=Monday, ..., 7=Sunday) if needed
    // dayOfWeek = (dayOfWeek == 0) ? 7 : dayOfWeek;
    
    return true;
  }

  /**
   * Check if time has been synchronized
   */
  bool isTimeSet() {
    return timeSet;
  }

  /**
   * Handle periodic NTP synchronization
   * Should be called regularly in the main loop
   */
  void handleSync() {
    if (WiFi.isConnected() && !syncInProgress) {
      syncTime();
    }
  }

  /**
   * Set timezone for local time calculation
   * Example: "CET-1CEST-2,M3.5.0/2,M10.5.0/3" for Central European Time with DST
   */
  void setTimezone(const char *tzinfo) {
    setenv("TZ", tzinfo, 1);
    tzset();
    Serial.printf("[NTP] Timezone set to: %s\n", tzinfo);
  }

  /**
   * Get current timezone name (CET or CEST)
   */
  const char* getCurrentTimezoneName() {
    struct tm timeinfo;
    if (!getCurrentTime(timeinfo)) {
      return "UNKNOWN";
    }
    return timeinfo.tm_isdst > 0 ? "CEST (UTC+2)" : "CET (UTC+1)";
  }

  /**
   * Print current time and timezone info for debugging
   */
  void printTimeInfo() {
    if (!timeSet) {
      Serial.println("[NTP] Time not yet synchronized");
      return;
    }

    struct tm timeinfo;
    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);

    Serial.printf("[NTP] Current time: %04d-%02d-%02d %02d:%02d:%02d (%s)\n",
                  timeinfo.tm_year + 1900,
                  timeinfo.tm_mon + 1,
                  timeinfo.tm_mday,
                  timeinfo.tm_hour,
                  timeinfo.tm_min,
                  timeinfo.tm_sec,
                  getCurrentTimezoneName());
  }
};

