/**
 * NeoPixel Clock with NTP Synchronization
 * This script controls a NeoPixel LED ring to display the current time,
 * synchronized via NTP (Network Time Protocol).
 *
 * Created on: 04.11.2023
 * Last updated on: 14.11.2023 V1.1 by Constantino Tobio, Jr.
 *
 * Author: Alf Müller / purecrea gmbh Switzerland / www.bastelgarage.ch
 * Modified by Constantino Tobio, Jr. to support ESP32
 *
 * Copyright (c) 2023 Alf Müller. All rights reserved.
 * Redistribution, distribution, and modification of this script are allowed
 * under the condition of attribution and without any licensing fees.
 */
#include <WiFi.h>                           // support for ESP32
#include "WiFiManager.h"                    // https://github.com/tzapu/WiFiManager
#include <Timezone.h>                       // https://github.com/JChristensen/Timezone
#include <TimeLib.h>                        // https://github.com/PaulStoffregen/Time
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#define PIN 16                              //GPIO16 for ESP32-WROOM Mini, GPIO13 for ESP32-WROOM Dev, because those pins are next to +5V and GND on those boards
#define NUMPIXELS 120

// NeoPixel library initialization
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// NTP client initialization, no offset needed here so we're pulling UTC
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);

// Variable to store the last minute
int lastMinute = -1;

void setup() {
  pixels.begin();
  WiFiManager wifiManager;
  wifiManager.autoConnect("NeoPixelClock");
  timeClient.begin();
}

void loop() {
  // Update NTP time
  timeClient.update();

  // Get current time in UTC
  time_t utc = timeClient.getEpochTime();

  // Convert UTC time to local time
  time_t local = usET.toLocal(utc);

  // get localtime that was set for the proper TZ and get integers for hours and minutes
  struct tm *current = localtime(&local);
  int hours = current->tm_hour;         // get hours since midnight (0-23)
  int minutes = current->tm_min;        // get minutes passed after the hour (0-59)

  // Set the LEDs based on the current time
  setTimeOnLEDs(hours, minutes, 55, 0, 0, 0, 0, 55);  // Red for minutes, blue for hours - hint: R, G, B values can be 0-255 here.
  delay(1000);                                        // Wait one second before the next iteration begins
}

void setTimeOnLEDs(int hours, int minutes, byte rMinutes, byte gMinutes, byte bMinutes, byte rHours, byte gHours, byte bHours) {
  pixels.clear();  // Turn off all LEDs

  // Set the minute LEDs
  for (int i = 0; i < minutes; i++) {
    int minutesLedIndex = (i + 29) % 60;                                               // Start at LED 29 for the first minute
    pixels.setPixelColor(minutesLedIndex, pixels.Color(rMinutes, gMinutes, bMinutes));  // Set color for each minute
  }

  // Calculate the number of LEDs that should be on for the hours
  // Make sure hours are in 12-hour format
  hours = hours % 12;
  hours = hours ? hours : 12;                // Convert 0 hours (midnight) to 12
  int hoursCount = hours * 5 + minutes / 12;  // Each hour has 5 LEDs, each minute adds 1/12 of an hour

  // Set the hour LEDs
  for (int i = 0; i < hoursCount; i++) {
    int hoursLedIndex = (90 - i) % 60 + 60;                                           // Start at LED 91 for the first hour
    pixels.setPixelColor(hoursLedIndex, pixels.Color(rHours, gHours, bHours));          // Set color for hours
  }

  pixels.show();  // Update the strip to show the changes
}