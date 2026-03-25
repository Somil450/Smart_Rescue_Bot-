#include <BluetoothSerial.h>
#include <DHT.h>
#include <ESP32Servo.h>

BluetoothSerial SerialBT;

// MOTOR PINS
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14

// ULTRASONIC
#define TRIG 5
#define ECHO 18

// SERVO
#define SERVO_PIN 4
Servo scanServo;

// GAS SENSOR
#define GAS_PIN 34

// FLAME SENSOR
#define FLAME_PIN 35

// DHT SENSOR
#define DHTPIN 21
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

enum Motion { STOPPED, FORWARD, BACKWARD };
Motion currentMotion = STOPPED;

const int SAFE_DISTANCE = 20;

////////////////////////////////////////////////////////

void setup() {

  Serial.begin(115200);
  SerialBT.begin("SmartBot");

  Serial.println("SMART RESCUE BOT READY");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(FLAME_PIN, INPUT);

  // Servo initialization
  scanServo.setPeriodHertz(50);
  scanServo.attach(SERVO_PIN, 500, 2400);
  scanServo.write(90);

  dht.begin();

  stopMotors();
}

////////////////////////////////////////////////////////

void loop() {

  readBluetooth();

  long distance = getDistance();
  int gasValue = analogRead(GAS_PIN);
  int flameValue = digitalRead(FLAME_PIN);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  printSensorData(distance, gasValue, flameValue, temperature, humidity);
  sendDataToApp(distance, gasValue, flameValue, temperature, humidity);

  // SAFETY CHECK
  if (gasValue > 2000 || flameValue == LOW || temperature > 45) {

    Serial.println("DANGER DETECTED → STOP");
    stopMotors();
    return;
  }

  // OBSTACLE DETECTION
  if (distance < SAFE_DISTANCE && currentMotion == FORWARD) {

    Serial.println("Obstacle Detected");

    stopMotors();
    delay(300);

    long leftDistance = checkLeft();
    long rightDistance = checkRight();

    Serial.print("Left Distance: ");
    Serial.println(leftDistance);

    Serial.print("Right Distance: ");
    Serial.println(rightDistance);

    if (leftDistance > rightDistance) {

      Serial.println("Turning LEFT");
      turnLeft();
      delay(350);

    } else {

      Serial.println("Turning RIGHT");
      turnRight();
      delay(350);
    }

    stopMotors();
    currentMotion = FORWARD;

    return;
  }

  // NORMAL MOVEMENT

  if (currentMotion == FORWARD)
    moveForward();

  else if (currentMotion == BACKWARD)
    moveBackward();

  else
    stopMotors();
}

////////////////////////////////////////////////////////
//////////////// BLUETOOTH CONTROL /////////////////////
////////////////////////////////////////////////////////

void readBluetooth() {

  if (SerialBT.available()) {

    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();

    Serial.print("Command: ");
    Serial.println(cmd);

    if (cmd == "forward")
      currentMotion = FORWARD;

    else if (cmd == "back")
      currentMotion = BACKWARD;

    else if (cmd == "left") {

      turnLeft();
      delay(300);
      stopMotors();
    }

    else if (cmd == "right") {

      turnRight();
      delay(300);
      stopMotors();
    }

    else if (cmd == "stop") {

      currentMotion = STOPPED;
      stopMotors();
    }
  }
}

////////////////////////////////////////////////////////
//////////////// ULTRASONIC SENSOR /////////////////////
////////////////////////////////////////////////////////

long getDistance() {

  long total = 0;

  for (int i = 0; i < 5; i++) {

    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    long duration = pulseIn(ECHO, HIGH, 30000);

    long distance = duration * 0.034 / 2;

    if (distance == 0)
      distance = 400;

    total += distance;

    delay(20);
  }

  return total / 5;
}

////////////////////////////////////////////////////////
//////////////// SERVO SCANNING ////////////////////////
////////////////////////////////////////////////////////

long checkLeft() {

  Serial.println("Scanning LEFT");

  scanServo.write(150);
  delay(700);

  long d = getDistance();

  scanServo.write(90);

  return d;
}

long checkRight() {

  Serial.println("Scanning RIGHT");

  scanServo.write(30);
  delay(700);

  long d = getDistance();

  scanServo.write(90);

  return d;
}

////////////////////////////////////////////////////////
//////////////// SENSOR DEBUG //////////////////////////
////////////////////////////////////////////////////////

void printSensorData(long d, int gas, int flame, float temp, float hum) {

  Serial.println("------ SENSOR DATA ------");

  Serial.print("Distance: ");
  Serial.println(d);

  Serial.print("Gas: ");
  Serial.println(gas);

  Serial.print("Flame: ");
  Serial.println(flame);

  Serial.print("Temp: ");
  Serial.println(temp);

  Serial.print("Humidity: ");
  Serial.println(hum);

  Serial.println("-------------------------");
}

////////////////////////////////////////////////////////
//////////////// DATA TO APP ///////////////////////////
////////////////////////////////////////////////////////

void sendDataToApp(long d, int gas, int flame, float temp, float hum) {

  SerialBT.println("DIST:" + String(d));
  SerialBT.println("GAS:" + String(gas));
  SerialBT.println("FLAME:" + String(flame));
  SerialBT.println("TEMP:" + String(temp));
  SerialBT.println("HUM:" + String(hum));
}

////////////////////////////////////////////////////////
//////////////// MOTOR CONTROL /////////////////////////
////////////////////////////////////////////////////////

void moveForward() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void moveBackward() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnLeft() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnRight() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void stopMotors() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}