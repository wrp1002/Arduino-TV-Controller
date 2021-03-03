#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

#define HOSTNAME "tv"
#define INPUT_PIN D5
#define OUTPUT_PIN D6
#define OUTPUT_DELAY 500

const char *ssid = "SSID";
const char *pass = "PASSWORD";
bool targetPowered = false;
bool targetReached = true;
int checkTimer = 500;
int nextTime = 0;

ESP8266WebServer server(80);


// ======================= Misc Functions =======================
String GetStatus() {
  StaticJsonDocument<20> doc;
  String responseHTML;

  bool powered = digitalRead(INPUT_PIN);
  doc["powered"] = powered;

  serializeJson(doc, responseHTML);

  return responseHTML;
}


// ======================= Setup =======================
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(INPUT_PIN, INPUT);

  digitalWrite(OUTPUT_PIN, HIGH);

  //  Start serial and wait for it to be ready
  Serial.begin(9600);
  while (!Serial);
  delay(250);

  Serial.println();
  Serial.println("ESP8266 Web Server Initializing...");
  Serial.println();

  Serial.println("Starting WiFi... ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  
  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(500);
  }
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("");
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Router IP: ");
  Serial.println(WiFi.gatewayIP().toString());
  Serial.print("mDNS Address: http://");
  Serial.print(HOSTNAME);
  Serial.println(".local");


  // ======================= ArduinoOTA =======================
  Serial.print("Starting ArduinoOTA... ");
  ArduinoOTA.setHostname(HOSTNAME);
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
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Done");


  // ======================= Web Interface =======================
  Serial.print("Setting up web interface...");
  server.on("/", HTTP_GET, HandleRoot);
  server.on("/status", HTTP_GET, HandleStatus);
  server.on("/on", HTTP_GET, HandleOn);
  server.on("/off", HTTP_GET, HandleOff);
  server.on("/timed", HTTP_GET, HandleTimed);
  server.on("/toggle", HTTP_GET, HandleToggle);
  server.onNotFound(HandleNotFound);

  server.begin();
  Serial.println("Done");
  //  ===========================================================================


  MDNS.addService("http", "tcp", 80);
  Serial.println("Setup Done!");
}

void loop() {
  MDNS.update();
  server.handleClient();
  ArduinoOTA.handle();

  if (millis() > nextTime) {
  	nextTime = millis() + checkTimer;
  	if (!targetReached) {
	  	if (targetPowered != digitalRead(INPUT_PIN))
	  		TogglePower();
	  	else
	  		targetReached = true;
	}
  }
}
