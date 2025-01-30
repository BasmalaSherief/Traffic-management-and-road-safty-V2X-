#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define the total number of motors and motor phases
#define Total_Motors 2
#define Motor_Phase 2

// Define the GPIO pins for motor control
#define M1_IN1 25
#define M1_IN2 26
#define M2_IN1 27
#define M2_IN2 14
#define ENA 12  // Enable pin for Motor 1
#define ENB 13  // Enable pin for Motor 2

#define IN1 0
#define IN2 1

typedef enum {
  Motor1,
  Motor2
} motor_n;

unsigned char motorArr[Total_Motors][Motor_Phase] = {
  {M1_IN1, M1_IN2},
  {M2_IN1, M2_IN2},
};

const char* ssid = "Bosey"; // WiFi SSID
const char* password = "B!b12345"; // WiFi Password

// MQTT broker details
const char* mqttServer = "172.20.10.2"; // MQTT broker IP address
const int mqttPort = 1883; // MQTT broker port
const char* mqttTopic = "Traffic/RoadStatus"; // MQTT topic to subscribe to

WiFiClient espClient;
PubSubClient client(espClient);


void setMotorSpeed(motor_n motorNumber, int speed) {
  if (motorNumber == Motor1) {
    analogWrite(ENA, speed); 
  } else if (motorNumber == Motor2) {
    analogWrite(ENB, speed); 
  }
}

void motorMoveForward(motor_n motorNumber)
{
  digitalWrite(motorArr[motorNumber][IN1],HIGH);
  digitalWrite(motorArr[motorNumber][IN2],LOW);
}

void motorStop(motor_n motorNumber)
{
  digitalWrite(motorArr[motorNumber][IN1],LOW);
  digitalWrite(motorArr[motorNumber][IN2],LOW);

}


void setup_wifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.println("Message: " + msg);

  if (String(topic) == "Traffic/RoadStatus") {
   if (msg == "TrafficJam") {
      Serial.println("Traffic Jam Detected");
      Serial.println("Take another route");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Jam Detected!");
      lcd.setCursor(0, 2);
      lcd.print("Change Route");
      setMotorSpeed(Motor1, 255); 
      setMotorSpeed(Motor2, 255); 
      motorMoveForward(Motor1);   
      motorMoveForward(Motor2);   
    } else if (msg == "Accident") {
      Serial.println("Accident Detected");
      Serial.println("Lowering speed for both motors");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Crash Detected!");
      lcd.setCursor(0, 2);
      lcd.print("Lowering speed");
      setMotorSpeed(Motor1, 128); 
      setMotorSpeed(Motor2, 128); 
      motorMoveForward(Motor1);   
      motorMoveForward(Motor2);   
    } else if (msg == "Low") {
      Serial.println("Low Traffic");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Low Traffic");
      lcd.setCursor(0, 2);
      lcd.print("Normal Speed");
      setMotorSpeed(Motor1, 255); 
      setMotorSpeed(Motor2, 255); 
      motorMoveForward(Motor1);   
      motorMoveForward(Motor2);   
    } else if (msg == "Medium") {
      Serial.println("Medium Traffic");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Medium Traffic");
      lcd.setCursor(0, 2);
      lcd.print("Normal Speed");
      setMotorSpeed(Motor1, 255); 
      setMotorSpeed(Motor2, 255); 
      motorMoveForward(Motor1);   
      motorMoveForward(Motor2);   
    } else if (msg == "High") {
      Serial.println("High Traffic");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("High Traffic");
      lcd.setCursor(0, 2);
      lcd.print("Normal Speed");
      setMotorSpeed(Motor1, 255); 
      setMotorSpeed(Motor2, 255); 
      motorMoveForward(Motor1);   
      motorMoveForward(Motor2);   
    } else {
      Serial.println("Unknown State");
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Detecting!");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32_client1")) {
      Serial.println("Connected to MQTT broker.");
      if (client.subscribe(mqttTopic)) {
        Serial.println("Subscribed to topic: " + String(mqttTopic));
      } else {
        Serial.println("Failed to subscribe.");
      }
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize the LCD display
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Initializing...");

  // Set motor control pins as outputs
  pinMode(ENA, OUTPUT);
  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);

  // Set initial motor speed and direction
  setMotorSpeed(Motor1, 255); 
  setMotorSpeed(Motor2, 255); 
  motorMoveForward(Motor1);   
  motorMoveForward(Motor2);
  
  setup_wifi();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
