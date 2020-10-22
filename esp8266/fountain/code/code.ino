#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>

#ifndef STASSID
#define STASSID "Gryzzl Wi-Fi"
#define STAPSK  "a817p948t44383426"
#define STAHOST "esp-0003"
#endif
#define LED_PIN 2
#define LED_COUNT 2
#define VERSION "0.1.2"

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN);

const int led = LED_BUILTIN;
int isOn = 0;
int brightness = 255;
int color[3] = {0,0,0};

String alert(String message, String type) {
  return "<div class=\"mb-4 flash" + (type ? " flash-" + type : "") + "\">" + message + "</div>";
}

bool noColor() {
  return color[0] == 0 && color[1] == 0 && color[2] == 0;
}

String colorLabel() {
  if (noColor()) {
    return "Select a color";
  }
  return "rgb(" + String(color[0]) + "," + String(color[1]) + "," + String(color[2]) + ")";
}


String getPage(String message) {
  return "<html>\
  <head>\
    <title>Fountain Controller</title>\
    <link href=\"https://unpkg.com/@primer/css/dist/primer.css\" rel=\"stylesheet\" />\
    <script src=\"https://unpkg.com/vanilla-picker@2\"></script>\
    <script>\
      document.addEventListener('DOMContentLoaded', function() {\
        var pickers = document.querySelectorAll('.p');\
        for (var i = 0; i < pickers.length; i++) {\
          var button = pickers[i].getElementsByTagName('button')[0];\
          var inputs = pickers[i].getElementsByTagName('input');\
          var picker = new Picker({ parent: button, alpha: false, popup: 'bottom' });\
          button.addEventListener('click', function() { picker.show() });\
          picker.onDone = function(color) {\
            console.log(color);\
            button.innerText = \'rgb(\' \+ color.rgba[0] \+ \',\' \+ color.rgba[1] \+ \',\' \+ color.rgba[2] \+ \')\';\
            button.style.backgroundColor = color.hex;\
            inputs[0].value = color.rgba[0];\
            inputs[1].value = color.rgba[1];\
            inputs[2].value = color.rgba[2];\
          }\
        }\
      });\
    </script>\
  </head>\
  <body>\
    <div class=\"container-sm mt-4 mb-4 clearfix\">\
      " + (message != "" ? alert(message, "success") : "") + "\
      <form method=\"post\" class=\"Box col-12\" action=\"/\">\
        <div class=\"Box-header d-flex flex-justify-between\">\
          <h3 class=\"Box-title\">Fountain Controls</h3>\
          <span class=\"branch-name\">v" + VERSION + "</span>\
        </div>\
        <div class=\"Box-body\">\
          <div class=\"form-checkbox\">\
            <label>\
              <input name=\"on\" type=\"checkbox\"" + (isOn ? " checked=\"checked\"" : "") + "/>Turn On?</label>\
          </div>\
          <div class=\"form-group\">\
            <div class=\"form-group-header\">\
              <label for=\"brightness\">Brightness</label>\
            </div>\
            <div class=\"form-group-body\">\
              <input class=\"input-block form-control\" type=\"range\" min=\"0\" max=\"255\" value=\"" + brightness + "\" name=\"brightness\"/>\
            </div>\
          </div>\
          <div class=\"form-group\">\
            <div class=\"form-group-header\">\
              <label>Color</label>\
            </div>\
            <div class=\"form-group-body\">\
              <div class=\"p\">\
                <button class=\"btn\" type=\"button\"" + String(noColor() ? "" : " style=\"background-color: rgb(" + String(color[0]) + "," + String(color[1]) + "," + String(color[2]) + ");\"") + ">" + colorLabel() + "</button>\
                <input type=\"hidden\" name=\"r\" value=\"" + color[0] + "\"/>\
                <input type=\"hidden\" name=\"g\" value=\"" + color[1] + "\"/>\
                <input type=\"hidden\" name=\"b\" value=\"" + color[2] + "\"/>\
              </div>\
            </div>\
          </div>\
        </div>\
        <div class=\"Box-footer\">\
          <div class=\"form-actions\">\
            <button class=\"btn btn-primary\" type=\"submit\">Submit</button>\
          </div>\ 
        </div>\
      </form>\
     </div>\
  </body>\
</html>";
}

void handleRoot() {
  if (server.method() == HTTP_GET) {
    server.send(200, "text/html", getPage(""));
  } else if (server.method() == HTTP_POST) {
    isOn = server.hasArg("on");
    String message = "Success. Turned LEDs " + String(isOn ?  "on" : "off") + ".";
    if (server.arg("r") != "") {
      color[0] = server.arg("r").toInt();
      color[1] = server.arg("g").toInt();
      color[2] = server.arg("b").toInt();
      message += " Set Color to rgb(" + server.arg("r") + "," + server.arg("g") + "," + server.arg("b") + ").";
    }
    brightness = server.arg("brightness").toInt();
    message += " Set Brightness to " + String(int(brightness / 255.0 * 100)) + "%.";
    updateStrip();
    server.send(200, "text/html", getPage(message));
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void updateStrip() {
  if (isOn) {
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, color[0], color[1], color[2]);
    }
    strip.setBrightness(brightness);
  } else {
    strip.clear();
  }
  strip.show();
}

void setup() {
  strip.begin();
  updateStrip();
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound); 
  server.begin();

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
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
