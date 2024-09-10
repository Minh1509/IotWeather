#include "DHT.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Loi"
#define WIFI_PASSWORD "0976300109"

// Raspberri Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 1, 29)
#define MQTT_PORT 1883

// Topic cho dữ liệu sensor
#define MQTT_PUB_SENSOR "datasensor"

// Digital pin connected to the DHT sensor
#define DHTPIN 14

// Pin connected to the light sensor
#define LIGHT_SENSOR_PIN A0 // Giả sử cảm biến ánh sáng được kết nối với chân analog A0

// Khai báo chân GPIO cho LED
#define LED1_PIN D2
#define LED2_PIN D7

// Uncomment whatever DHT sensor type you're using
#define DHTTYPE DHT11   // DHT 11
// #define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)   

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Variables to hold sensor readings
float temp;
float hum;
float lux; // Thay đổi biến để lưu giá trị ánh sáng thành float

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

#define MQTT_USERNAME "root"
#define MQTT_PASSWORD "password"

// Constants for LDR calculation
const float referenceVoltage = 3.3; // Điện áp tham chiếu

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  // Subscribe to topic for controlling LEDs
  mqttClient.subscribe("controldevice", 1);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

// Callback function for handling MQTT messages
void onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total) {
  Serial.println("Message arrived [" + String(topic) + "]");

  // Chuyển đổi payload thành chuỗi
  String message(payload);
  
  // Phân tích cú pháp JSON
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, message);

  String device = doc["device"];
  String status = doc["status"];

  // Điều khiển LED dựa trên thông tin JSON
  if (device == "LED1") {
    digitalWrite(LED1_PIN, (status == "On") ? HIGH : LOW);
  } else if (device == "LED2") {
    digitalWrite(LED2_PIN, (status == "On") ? HIGH : LOW);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  dht.begin();
  
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.onMessage(onMessage); // Đăng ký callback xử lý tin nhắn

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
  
  connectToWifi();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    int rawLight = analogRead(LIGHT_SENSOR_PIN); // Đọc giá trị từ cảm biến ánh sáng
    float voltage = rawLight * (referenceVoltage / 1023.0); // Chuyển đổi giá trị thành điện áp
    
    // Giả sử một công thức đơn giản để chuyển đổi điện áp sang lux
    lux = voltage * 1000; // Chuyển đổi giá trị điện áp thành lux (giả sử 1V = 1000 lux)
    
    Serial.printf("Raw light value: %d\n", rawLight); // In giá trị đọc từ cảm biến ánh sáng
    Serial.printf("Converted light value (lux): %.2f\n", lux); // In giá trị ánh sáng sau khi chuyển đổi
    
    // Tạo đối tượng JSON
    DynamicJsonDocument doc(1024);
    doc["temperature"] = temp;
    doc["humidity"] = hum;
    doc["light"] = lux;

    String jsonString;
    serializeJson(doc, jsonString);

    uint16_t packetId = mqttClient.publish(MQTT_PUB_SENSOR, 1, true, jsonString.c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_SENSOR, packetId);
    Serial.printf("Message: %s \n", jsonString.c_str());
  }

  // mqttClient.loop(); // Không cần gọi khi sử dụng AsyncMqttClient
}
