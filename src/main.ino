/*
   Project: ESP_LED
   Version: 0.3
   Author: Antoine de Chassey
   Date: 04/26/2017

   Dependencies:
   - https://github.com/esp8266/Arduino
   - https://github.com/esp8266/arduino-esp8266fs-plugin
   - https://github.com/bblanchon/ArduinoJson
   - https://github.com/knolleary/pubsubclient
   - https://github.com/FastLED/FastLED

   Hardware setup:
   GND ---------------------- GND
   VDD ---------------------- 5V
   DATA --------------------- D1
 */

// SPIFFS
#include <FS.h>
// JSON
#include <ArduinoJson.h>
// FastLED
#include <FastLED.h>
// ESP
#include <ESP8266WiFi.h>          // ESP8266 library
#include <PubSubClient.h>         // MQTT library
//#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
//#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal

// LEDs
#define LED_PIN     2
#define NUM_LEDS    30
#define BRIGHTNESS  255
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define TEMPERATURE_1 Tungsten100W
#define TEMPERATURE_2 OvercastSky
// How many seconds to show each temperature before switching
#define DISPLAYTIME 20
// How many seconds to show black between switches
#define BLACKTIME   3

// Variables
const unsigned long onMaxTime = 1000*60*120;  // Time before auto led off (2h)
unsigned long previousMillis = 0;
bool isLedOn      =    true;
bool isRainbow    =    true;
int R,G,B = 255;
char clientID[20] =     "";       // Used to hold ESP chip ID (useful while connecting to the MQTT broker)

// Configuration file read in SPIFFS (stored in /data/config.json)
struct ConfigData {
        String name;   // The app name
        // WiFi setup
        String wifi_ssid;
        String wifi_password;
        // MQTT setup
        String mqtt_server;
        int mqtt_port;
        String mqtt_username;     // Used while connecting to the broker
        String mqtt_password;     // Leave empty if not needed
        String mqtt_topic_publish;
        String mqtt_topic_subscribe; // Used to receive incomming commands
        String mqtt_topic_metadata; // Used to log the hardware status to broker
} configData;

WiFiClient espClient;
PubSubClient client(espClient);

/* ================================================================
                               SETUP
   ================================================================ */
void setup()
{
        Serial.begin(115200);
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, LOW); // Turn the LED on

        // Read config.json stored in SPIFFS memory
        if (!SPIFFS.begin()) {
                Serial.println("Failed to mount file system");
                return;
        }
        if (!loadConfig()) {
                Serial.println("Failed to load config");
                return;
        } else {
                Serial.println("Config loaded");
        }
        Serial.println("\n-------------------------");
        Serial.println("MQTT : "); Serial.println(configData.mqtt_topic_metadata);
        Serial.println("-------------------------\n");

        /* ================================================================
                                     WIFI SETUP
           ================================================================ */
        sprintf(clientID,"%lu", ESP.getChipId());
        Serial.print(F("ESP ID: "));
        Serial.println((const char*) clientID);

        Serial.print(F("Trying to connect to access point..."));
        WiFi.mode(WIFI_STA);
        WiFi.begin(configData.wifi_ssid.c_str(), configData.wifi_password.c_str());
        // Trying to connect to default AP. If connection isn't established after 15 seconds, IMU connects to RPi
        while ((WiFi.status() != WL_CONNECTED)) {
                delay(500);
                Serial.print(".");
        }

        Serial.println("");
        Serial.print("WiFi connected on: "); Serial.println(WiFi.SSID());
        Serial.print("IP address: "); Serial.println(WiFi.localIP());

        /*Serial.printf("\nConnection status: %d\n", WiFi.status());
           WiFi.setAutoReconnect(true); */
        client.setServer(configData.mqtt_server.c_str(), configData.mqtt_port);
        Serial.println("MQTT Server: " + String(configData.mqtt_server) + " on port: " + String(configData.mqtt_port));

        // Set the MQTT callback to control received payloads
        client.setCallback(callback);

        // Send MQTT connection status to broker
        connectMQTT();

        /* ================================================================
                                     FastLED
           ================================================================ */
        FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
        FastLED.setBrightness(  BRIGHTNESS );

        // Turn the LED off
        digitalWrite(LED_BUILTIN, HIGH);

}


/* ================================================================
                               LOOP
   ================================================================ */
void loop() {
        unsigned long currentMillis = millis();
        if ((currentMillis - previousMillis) <= onMaxTime) {
                if(isRainbow)
                        showRainbow();
        } else {
                previousMillis = currentMillis;
                ledOff();
        }
        client.loop();
}
