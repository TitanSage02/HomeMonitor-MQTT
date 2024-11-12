// WiFi & MQTT Libraries
#include <WiFi.h>
#include <PubSubClient.h>

// Salon S
#define LED_S 12
#define INT_S 13

// Chambre C
#define LED_C 4
#define INT_C 15

// WiFi Access
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// MQTT Config
#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883 

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Variables to track LED states
bool ledStateS = false;
bool ledStateC = false;

void setup_wifi() {
  WiFi.begin(ssid, pass);
  Serial.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(10);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
}

void setup_mqtt() {
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  Serial.println("Connecting to MQTT broker...");
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP_Client")) {
      Serial.println("Connected to MQTT broker!");
      mqttClient.subscribe("salon/interrupt");    // Subscription for Salon LED control
      mqttClient.subscribe("chambre/interrupt");  // Subscription for Chambre LED control
    } else {
      Serial.print(".");
      delay(20);
    }
  }
}

// Callback function called when a message is received on subscribed topics
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message received on topic ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  // Control LEDs based on MQTT messages
  if (String(topic) == "salon/interrupt") {
    ledStateS = (message == "1");  // Convert message to boolean for LED control
    digitalWrite(LED_S, ledStateS ? HIGH : LOW);
    Serial.println(ledStateS ? "Salon LED turned ON" : "Salon LED turned OFF");
  }
   else if (String(topic) == "chambre/interrupt") {
    ledStateC = (message == "1");
    digitalWrite(LED_C, ledStateC ? HIGH : LOW);
    Serial.println(ledStateC ? "Chambre LED turned ON" : "Chambre LED turned OFF");
  }
}

void setup() {
  // Configure pin modes
  pinMode(LED_S, OUTPUT);
  pinMode(INT_S, INPUT_PULLUP);

  pinMode(LED_C, OUTPUT);
  pinMode(INT_C, INPUT_PULLUP);

  // Serial for debug
  Serial.begin(115200);

  // Initialize WiFi and MQTT
  setup_wifi();
  setup_mqtt();
}

void loop() {
  // Ensure MQTT connection is maintained
  if (!mqttClient.connected()) {
    setup_mqtt();
  }
  mqttClient.loop();

  // Read button states
  bool buttonStateS = !digitalRead(INT_S);  // Button pressed = LOW
  bool buttonStateC = !digitalRead(INT_C);

  // Control Salon LED with physical button and publish state
  if (buttonStateS != ledStateS) {  // Detect button press change
    ledStateS = buttonStateS;
    mqttClient.publish("salon/interrupt", ledStateS ? "1" : "0");  // Publish state change
    Serial.println(ledStateS ? "Salon LED turned ON via button" : "Salon LED turned OFF via button");
  }

  // Control Chambre LED with physical button and publish state
  if (buttonStateC != ledStateC) {
    ledStateC = buttonStateC;
    mqttClient.publish("chambre/interrupt", ledStateC ? "1" : "0");
    Serial.println(ledStateC ? "Chambre LED turned ON via button" : "Chambre LED turned OFF via button");
  }

  delay(100);  // Small delay to avoid bouncing
}