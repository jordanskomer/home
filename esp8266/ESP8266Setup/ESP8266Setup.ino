/*
 * ESP8266Setup.h - Internal Library for setting up ESP8266 wth the following.
 * 1. OTA Updates
 * 2. Websocket enabled + Handler
 * 3. WiFi STA
 */
#ifndef ESP8266Setup_h
#define ESP8266Setup_h
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
class ESP8266Setup
{
  public ESP8266Setup(char* ssid, char* pass, char* host, int v = 1) {
    _verbose = v;
    wifi(ssid, pass);
    ota(host);
  }

  public void ESP8266::loop() {
    ArduinoOTA.handle();
  }

  private int _verbose;

  private void print(char* meth, char* mess) {
    if (_verbose) {
      Serial.println("ESP8266." + meth + ": " + m);
    }
  }
  
  private void wifi(char* ssid, char* pass) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      print("wifi", "Connection Failed! Rebooting...");
      delay(5000);
      ESP.restart();
    }
    print("wifi", "Ready. IP address: " + WiFi.localIP());
  }

  private void ota(char* host) {
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);
  
    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(STAHOST);
  
    // No authentication by default
    // ArduinoOTA.setPassword("admin");
  
    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_FS
        type = "filesystem";
      }
  
      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      print("ota", "Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      print("ota", "\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      print("ota", "Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      print("ota", "Error[%u]: ", error);
    });
    ArduinoOTA.begin();
  }
}
