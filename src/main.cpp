#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <RCSwitch.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define SSID      ""
#define SSID_PW   ""
#define HOSTNAME  "MQTTto433MHz"

#define TRANSMITTER_PIN D1
#define MQTT_BROKER "home.lan"



WiFiClient espClient;
PubSubClient client(espClient);

StaticJsonDocument<200> doc;

//ESPiLight rf(TRANSMITTER_PIN);
RCSwitch mySwitch = RCSwitch();

void setup_wifi() {
  Serial.print("Connecting");
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(SSID, SSID_PW);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("OK:");
  Serial.println("IP: ");
  Serial.print(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg;

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg += (char)payload[i];
  }
  DeserializationError error = deserializeJson(doc, msg);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  Serial.println();

  String state = doc["State"];
  //String homecode = doc["Homecode"];
  //int device = doc["Device"];
  const char* homecode = doc["Homecode"];
  const char* device = doc["Device"];
  const char* outTopic = HOSTNAME ;
  //+ "/" + homecode + "/" + device;

  if (state == "On") {
    mySwitch.switchOn(homecode, device);
    client.publish(outTopic, "On");
  }
  if (state == "Off"){
    mySwitch.switchOff(homecode, device);
    client.publish(outTopic, "Off");
    //client.publish()
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = HOSTNAME;
    clientId += "-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  mySwitch.enableTransmit(TRANSMITTER_PIN);
  Serial.begin(115200);
  //Serial.begin(9600);
  while (!Serial) continue;
  delay(500);

  setup_wifi();

  client.setServer(MQTT_BROKER, 1883);
  client.setCallback(callback);
}

void loop() {
/*   Serial.println("Dose AN");
  //mySwitch.switchOn("10000", 2);
  delay(3000);
  Serial.println("Dose AUS");
  //mySwitch.switchOff("10000", 2);
  delay(3000); */

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(1000);
}