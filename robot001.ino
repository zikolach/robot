#include <AFMotor.h>
#include <NewPing.h>
#include <Servo.h> 

// Motors control

AF_DCMotor motors[4] = { AF_DCMotor(1), AF_DCMotor(2), AF_DCMotor(3), AF_DCMotor(4) };

#define LEFT 3
#define RIGHT 4

void move(int dir, int speed) {
  for (int i = 0; i < sizeof(motors) / sizeof(*motors); i++) {
    switch (dir) {
      case FORWARD:
      case BACKWARD:
        motors[i].run(dir);
        break;
      case RIGHT:
        if (i == 0 || i == 3)
          motors[i].run(FORWARD);
        else
          motors[i].run(BACKWARD);
        break;
      case LEFT:
        if (i == 0 || i == 3)
          motors[i].run(BACKWARD);
        else
          motors[i].run(FORWARD);
        break;
      default:
        motors[i].run(RELEASE);
    }
    motors[i].setSpeed(speed);
  }
}

void forward() {
  move(FORWARD, 255);
}

void stop() {
  move(RELEASE, 0);
}

void left() {
  move(LEFT, 255);
}

void right() {
  move(RIGHT, 255);
}

void backward() {
  move(BACKWARD, 255);
}

// Sensor ultrasonic

Servo myservo;
NewPing sonar(9, 9, 200);

const int initAngle = 100;
const int angles[] = {initAngle - 50, initAngle - 25, initAngle, initAngle + 25, initAngle + 50};
int dist[5];
byte med = sizeof(angles) / sizeof(*angles) / 2;

void lookAround(boolean rev = false) {
  myservo.attach(10);
  byte cnt = sizeof(angles) / sizeof(*angles);
  for (byte i = 0; i < cnt; i++) {
   byte a = rev ? cnt - i - 1 : i;
    myservo.write(angles[a]);
    delay(300);
    int d = sonar.ping() / US_ROUNDTRIP_CM;
    delay(100);
//    dist[a] = (dist[a] + d) / 2; //filter
    dist[a] = d; //filter
  }
  myservo.detach();
}

void lookForward() {
  myservo.attach(10);
  myservo.write(angles[med]);
  delay(100);
  int d = sonar.ping() / US_ROUNDTRIP_CM;
  delay(100);
//  dist[med] = (dist[med] + d) / 2; //filter
  dist[med] = d; //filter
  myservo.detach();
}
void setup() {
    Serial.begin(9600);
    calibrate();
}

void printAngleDist() {
  for (int i = 0; i < sizeof(dist) / sizeof(*dist); i++) {
    Serial.print(angles[i] - initAngle);
    Serial.print(":");
    Serial.println(dist[i]);
  }
}

void calibrateSpeed() {
  Serial.println("Calibrating speed...");
  int ms = 200, tries = 4;
  float speedForward = 0, speedBackward = 0;
  int d1, d2;
  for (int i = 0; i < tries; i++) {
    lookForward(); d1 = dist[med];
    backward();delay(ms);stop();
    lookForward(); d2 = dist[med];
    speedBackward = speedBackward == 0 ? (d2 - d1) * 1000 / ms : (speedBackward + (d2 - d1) * 1000 / ms) / 2;
    Serial.print(d2);Serial.print(" - ");Serial.println(d1);
    delay(ms);
    forward();delay(ms);stop();
    lookForward(); d1 = dist[med];
    speedForward = speedForward == 0 ? (d2 - d1) * 1000 / ms : (speedForward + (d2 - d1) * 1000 / ms) / 2;
    Serial.print(d2);Serial.print(" - ");Serial.println(d1);
  }
  Serial.print("Forward (cm/s): ");
  Serial.println(speedForward);
  Serial.print("Backward (cm/s): ");
  Serial.println(speedBackward);
}

float angle(float ab, float ac, float a) {
  return atan2((ac * sin(a / 180.0 * PI)), (ab - ac * cos(abs(a) / 180.0 * PI))) / PI * 180.0;
}

//void calibrateRotation() {
//  lookAround();
//  lookAround(true);
//  printAngleDist();
//  Serial.print("AngleR "); Serial.println(angle(dist[med], dist[1], angles[1] - initAngle));
//  Serial.print("AngleL "); Serial.println(angle(dist[med], dist[med * 2 - 1], angles[med * 2 - 1] - initAngle));
//}

void alg1() {
  lookForward();
  if (dist[med] > 40) {
    forward();
    delay(100);
  } else {
    backward();delay(50);stop();
    lookAround();
    if (dist[0] > dist[med * 2]) {
      right();
    } else {
      left();
    }
    delay(300);
    stop();
    lookForward();
  }
}


void calibrate() {
//  calibrateSpeed();
//  calibrateRotation();
}

void loop() {
  alg1();
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    switch (inChar) {
      case 'a':
        lookAround();
        lookAround(true);
        lookForward();
        printAngleDist();
        break;
      case 'f':
        forward();
        delay(100);
        stop();
        break;
      case 'b':
        backward();
        delay(100);
        stop();
        break;
      case 'l':
        left();
        delay(100);
        stop();
        break;
      case 'r':
        right();
        delay(100);
        stop();
        break;
    }
  }
}
