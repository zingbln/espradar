#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
extern "C" {
#include "user_interface.h"
}
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>

os_timer_t Timer2;
int Counter = 0;
bool TickOccured = false;

int radar = D2;
byte val = LOW;
int led = D1;
int i = 1023;

#define MAX_SRV_CLIENTS 1
const char* ssid = "YOURSSID";
const char* password = "WPAPASSWORD";
const char* host = "ESP-B38661";
const char* MQTT_BROKER = "192.168.1.1";

#define pir_topic "espradar/pir"
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

WiFiClient espClient;
PubSubClient client(espClient);

void timerCallback(void *pArg)
{ TickOccured = true;
  *((int *) pArg) += 1;
} 

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void handleRoot() {
  httpServer.send(200, "text/plain", "ESP8266WebServer");
}


void setup() 
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); //Client only
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  client.setServer(MQTT_BROKER, 1883);

  //dht.begin();
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  httpServer.on("/", handleRoot);
  
  TickOccured = false;
  os_timer_setfn(&Timer2, timerCallback, &Counter);
  os_timer_arm(&Timer2, 1000, true);

  pinMode (radar, INPUT); // Radar
  pinMode (led, OUTPUT); // Out
  pinMode(BUILTIN_LED, OUTPUT);
}

void loop() 
{
if (!client.connected()) {
    reconnect();
      }
   client.loop();
  httpServer.handleClient();
  int sensorValue = analogRead(A0);  //Fotowiderstand
  float voltage = sensorValue * (3.2 / 1023.0);
  Serial.println(voltage);

  val = digitalRead(radar);
  Serial.println(val, DEC);

  Serial.println(Counter);

  if (val == HIGH) {
    digitalWrite(BUILTIN_LED, LOW);
    client.publish(pir_topic, "true");
    if(voltage < 0.7) { 
            analogWrite(led, 1023);
            Serial.println("high");
            Counter = 0;
            }
    } 
   else {
        Serial.println("low");
        client.publish(pir_topic, "false");
        digitalWrite(BUILTIN_LED, HIGH);
    if (Counter == 240) {
            i=1023;
            while(i>0) {
            analogWrite(led, i);
            i--;
            delay(2);
            }
        }
        if (Counter > 240) {
            analogWrite(led, 0);
            }
        }
    delay(1000);
    }
