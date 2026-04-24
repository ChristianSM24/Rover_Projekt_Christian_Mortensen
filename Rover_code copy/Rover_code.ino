#include <DHT22.h>

#include <SPI.h>
#include <RF24.h>
#include <Servo.h>
#include <DHT.h>

// RF Module 
RF24 radio(48, 53);  // CE, CSN
const byte address[6] = "rover";

// DHT22 
#define DHT_PIN  22
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Motor Pins 
const int motor1pin1 = 8;
const int motor1pin2 = 7;
const int motor2pin1 = 5;
const int motor2pin2 = 4;
const int ena = 9;
const int enb = 3;

// Servo Pins
Servo shoulder;
Servo elbow;
Servo gripper;
const int SHOULDER_PIN = 11;
const int ELBOW_PIN    = 10;
const int GRIPPER_PIN  = 6;

// Servo Positions 
int shoulderPos = 90;
int elbowPos    = 90;

// Data packet to receive 
struct DataPacket {
  int joyX;
  int joyY;
  bool button;
};

struct ReceivePacket {
  float temperature;
  float humidity;
};

void setup() {
  // Motor pins
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  pinMode(ena, OUTPUT);
  pinMode(enb, OUTPUT);

  // Servos
  shoulder.attach(SHOULDER_PIN);
  elbow.attach(ELBOW_PIN);
  gripper.attach(GRIPPER_PIN);
  shoulder.write(shoulderPos);
  elbow.write(elbowPos);
  gripper.write(0);

   dht.begin();

  // RF
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();  // receiver mode

  Serial.begin(9600);
  Serial.println("Rover ready!");
}

void loop() {
  ReceivePacket response;
  response.temperature = dht.readTemperature();
  response.humidity    = dht.readHumidity();
  radio.writeAckPayload(0, &response, sizeof(response));
  
  if (radio.available()) {
    DataPacket data;
    radio.read(&data, sizeof(data));

    Serial.print("X: "); Serial.print(data.joyX);
    Serial.print("  Y: "); Serial.print(data.joyY);
    Serial.print("  BTN: "); Serial.println(data.button);

    // Drive motors 
    if (data.joyY > 0) {
      // Forward
      int speed = map(data.joyY, 0, 100, 100, 255);
      analogWrite(ena, speed);
      analogWrite(enb, speed);
      digitalWrite(motor1pin1, HIGH);
      digitalWrite(motor1pin2, LOW);
      digitalWrite(motor2pin1, LOW);
      digitalWrite(motor2pin2, HIGH);
    }
    else if (data.joyY < 0) {
      // Backward
      int speed = map(abs(data.joyY), 0, 100, 100, 255);
      analogWrite(ena, speed);
      analogWrite(enb, speed);
      digitalWrite(motor1pin1, LOW);
      digitalWrite(motor1pin2, HIGH);
      digitalWrite(motor2pin1, HIGH);
      digitalWrite(motor2pin2, LOW);
    }
    else if (data.joyX > 0) {
      // Turn right
      int speed = map(data.joyX, 0, 100, 100, 255);
      analogWrite(ena, speed);
      analogWrite(enb, speed);
      digitalWrite(motor1pin1, HIGH);
      digitalWrite(motor1pin2, LOW);
      digitalWrite(motor2pin1, HIGH);
      digitalWrite(motor2pin2, LOW);
    }
    else if (data.joyX < 0) {
      // Turn left
      int speed = map(abs(data.joyX), 0, 100, 100, 255);
      analogWrite(ena, speed);
      analogWrite(enb, speed);
      digitalWrite(motor1pin1, LOW);
      digitalWrite(motor1pin2, HIGH);
      digitalWrite(motor2pin1, LOW);
      digitalWrite(motor2pin2, HIGH);
    }
    else {
      // Stop
      analogWrite(ena, 0);
      analogWrite(enb, 0);
    }

    // Gripper 
    gripper.write(data.button ? 90 : 0);
  }
}

