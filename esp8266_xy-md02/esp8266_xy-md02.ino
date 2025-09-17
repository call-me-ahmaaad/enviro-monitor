/* --------------------------------------------------
Author    : Muhammad
GitHub    : @call-me-ahmaaad
Instagram : @just.type_miguel
LinkedIn  : linkedin.com/in/muhammad-ahmad-9a1857266

Function:
  - Reads temperature and humidity via Modbus (RS485)
  - Sends data to the MQTT broker in JSON format
  - Illuminates the WiFi, MQTT, and sensor status indicator LEDs

-------------------------------------------------- */

// ---------- Libraries ----------
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
// ---------- Libraries ----------

// ---------- Pin Configuration ----------
#define RX_PIN D2
#define TX_PIN D1

#define wifiLED D5
#define mqttLED D6
#define sensorLED D7
// ---------- Pin Configuration ---------- 

// ---------- WiFi Credential ----------
const char* ssid = "TYPE_YOUR_WIFI_SSID";
const char* password = "TYPE_YOUR_WIFI_PASSWORD";
// ---------- WiFi Credential ----------

// ---------- MQTT Configuration ----------
const char* mqtt_server = "TYPE_YOUR_MQTT_SERVER";
const int mqtt_port = "TYPE_YOUR_MQTT_PORT";

String TOPIC_INIT = "temp_humid";
String AREA = "packout";
// ---------- MQTT Configuration ----------

// ---------- Event Handlers & Objects ----------
Ticker wifiReconnectTimer;

WiFiClient espClient;
PubSubClient client(espClient);

SoftwareSerial RS485Serial(RX_PIN, TX_PIN);
ModbusMaster node;
// ---------- Event Handlers & Objects ----------

// ---------- Timer Section ----------
unsigned long previousMillis_pubMsg = 0;
unsigned long interval_pubMsg = 3000;

unsigned long previousMillis_reconnectMQTT = 0;
unsigned long interval_reconnectMQTT = 5000;

unsigned long previousMillis_getSensorData = 0;
unsigned long interval_getSensorData = 1000;
// ---------- Timer Section ----------

// ---------- Function to get data from sensor ----------
void getSensorData(float& temp, float& humid, bool& sensorStatus){
  uint8_t result;
  uint16_t data[2];

  result = node.readInputRegisters(0x0001, 2);

  if(result == node.ku8MBSuccess){
    data[0] = node.getResponseBuffer(0); // Temperature
    data[1] = node.getResponseBuffer(1); // Humidity

    temp = data[0] / 10.0;
    humid = data[1] / 10.0;

    digitalWrite(sensorLED, HIGH);
    sensorStatus = true;
  }else{
    Serial.print("Modbus Error: ");
    Serial.println(result);

    digitalWrite(sensorLED, LOW);
    sensorStatus = false;
  }
}

// ---------- Function to print data to Serial Monitor ----------
void printData(float temp, float humid, bool sensorStatus){
  Serial.println("====================");
  Serial.print("WiFi Status: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "0.0.0.0");

  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  Serial.print("Chip ID: ");
  Serial.println(getChipID());

  Serial.println();

  Serial.print("Sensor Status: ");
  Serial.println(sensorStatus ? "ACTIVE" : "INACTIVE");

  Serial.print("Temperature: ");
  Serial.println(temp);

  Serial.print("Humidity: ");
  Serial.println(humid);
  Serial.println("====================");
}

// ---------- Function to get device chip id ----------
String getChipID(){
  uint32_t chipid = ESP.getChipId();

  char id[9];
  sprintf(id, "%08X", chipid);

  return String(id);
}

// ---------- Function to connect device to WiFi AP ----------
void connectingToWiFi(){
  Serial.println("Connecting device to WiFi AP...");
  WiFi.begin(ssid, password);
}

// ---------- WiFi Event Callback Function ----------
void onStationModeConnected(const WiFiEventStationModeConnected& evt){
  Serial.println("Device connected to WiFi AP!");
}

void onStationModeGotIP(const WiFiEventStationModeGotIP& evt){
  Serial.print("WiFi SSID: ");
  Serial.println(WiFi.SSID());

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  wifiReconnectTimer.detach();
}

void onStationModeDisconnected(const WiFiEventStationModeDisconnected& evt){
  Serial.println("Device disconnected from WiFi AP!");
  Serial.print("Reason: ");
  Serial.println(evt.reason);

  wifiReconnectTimer.once(5, connectingToWiFi);
}
// ---------- WiFi Event Callback Function ----------

// ---------- Function to publish message to MQTT Topic ----------
void publishMQTT(String topic, String payload){
  bool publish = client.publish(topic.c_str(), payload.c_str());

  Serial.print("Pub. Status: ");
  Serial.println(publish == true ? "DELIVERED" : "FAILED");

  Serial.print("Payload: ");
  Serial.println(publish == true ? payload : "-");
}

// ---------- Function to reconnect to MQTT broker ----------
void reconnectMQTT(){
  while(!client.connected()){
    Serial.println("Attempting connection to MQTT...");
    String clientID = AREA + getChipID();

    if(client.connect(clientID.c_str())){
      Serial.println("Connected to MQTT");
    }else{
      Serial.print("Failed. RC: ");
      Serial.print(client.state());
      Serial.print(" . Try again in 5 seconds");
      delay(5000);
    }
  }
}

/* ---------------------------------------------------------------------------
  SETUP: Runs once at boot time
  - Serial, RS485, pin, WiFi, and MQTT initialization
--------------------------------------------------------------------------- */
void setup() {
  Serial.begin(115200);

  RS485Serial.begin(9600);
  node.begin(1, RS485Serial); // Modbus slave ID = 1, might be different with yours. Please check again.

  Serial.println("Setup starting...");

  pinMode(wifiLED, OUTPUT);
  pinMode(mqttLED, OUTPUT);
  pinMode(sensorLED, OUTPUT);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  delay(1000);

  WiFi.onStationModeConnected(onStationModeConnected);
  WiFi.onStationModeGotIP(onStationModeGotIP);
  WiFi.onStationModeDisconnected(onStationModeDisconnected);

  delay(1000);

  Serial.println("Start connecting device to WiFi AP...");
  WiFi.begin(ssid, password);

  delay(1000);

  Serial.println("Start connecting device to MQTT...");
  client.setServer(mqtt_server, mqtt_port);

  delay(1000);
}


/* ---------------------------------------------------------------------------
  LOOP: Runs repeatedly
  - Checks MQTT connection
  - Updates status LED
  - Reads sensor every 1 second
  - Publishes data every 3 seconds
--------------------------------------------------------------------------- */
void loop() {
  unsigned long currentMillis = millis();

  if(!client.connected()){
    digitalWrite(mqttLED, LOW);

    if(currentMillis - previousMillis_reconnectMQTT >= interval_reconnectMQTT){
      previousMillis_reconnectMQTT = currentMillis;
      reconnectMQTT();
    }
  }else{
    digitalWrite(mqttLED, HIGH);
  }
  client.loop(); 

  digitalWrite(wifiLED, WiFi.status() == WL_CONNECTED ? HIGH : LOW);

  float temp;
  float humid;
  bool sensorStatus;

  if(currentMillis - previousMillis_getSensorData >= interval_getSensorData){
    getSensorData(temp, humid, sensorStatus);
    previousMillis_getSensorData = currentMillis;
  }

  if(currentMillis - previousMillis_pubMsg >= interval_pubMsg){
    printData(temp, humid, sensorStatus);

    String topic = TOPIC_INIT + "/" + AREA + "/" + getChipID();

    StaticJsonDocument<256> doc;

    doc["ip"] = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "0.0.0.0";
    doc["mac"] = WiFi.macAddress();
    doc["chip_id"] = getChipID();
    doc["sensor_status"] = sensorStatus;
    doc["temperature"] = sensorStatus ? String(temp) : "N/A";
    doc["humidity"] = sensorStatus ? String(humid) : "N/A";

    String payload;
    serializeJson(doc, payload);

    publishMQTT(topic, payload);

    Serial.println();

    previousMillis_pubMsg = currentMillis;
  }
}
