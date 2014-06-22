#include <AFMotor.h>
#include <NewPing.h>
#include <Servo.h> 

// Motors control

AF_DCMotor motors[4] = { AF_DCMotor(1), AF_DCMotor(2), AF_DCMotor(3), AF_DCMotor(4) };

const double CD = 0.220 / 20.; // meters
int pinA0 = HIGH;
int pinA1 = HIGH;
volatile double speed0 = 0;
volatile double speed1 = 0;
volatile unsigned long m0 = millis();
volatile unsigned long m1 = millis();
int maxSpeedDelay = 200;

#define LEFT 3
#define RIGHT 4

int speed = 255;
boolean runAutopilot = false;

/* move robot in direction with speed */
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
  move(FORWARD, speed);
}

void stop() {
  move(RELEASE, 0);
}

void left() {
  move(LEFT, speed);
}

void right() {
  move(RIGHT, speed);
}

void backward() {
  move(BACKWARD, speed);
}

// Sensor ultrasonic

Servo myservo;
NewPing sonar(9, 9, 200);

const int initAngle = 100;
const int angles[] = {initAngle - 50, initAngle - 25, initAngle, initAngle + 25, initAngle + 50};
int dist[5];
byte med = sizeof(angles) / sizeof(*angles) / 2;

void radar(boolean rev = false) {
  myservo.attach(10);
  byte cnt = sizeof(angles) / sizeof(*angles);
  for (byte i = 0; i < cnt; i++) {
   byte a = rev ? cnt - i - 1 : i;
    myservo.write(angles[a]);
    delay(100);
    int d = sonar.ping() / US_ROUNDTRIP_CM;
    delay(100);
    dist[a] = d;
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
    InitialiseIO();
    InitialiseInterrupt();
}

/* print radar */
void printAngleDist() {
  for (int i = 0; i < sizeof(dist) / sizeof(*dist); i++) {
    Serial.print(angles[i] - initAngle);
    Serial.print(":");
    Serial.println(dist[i]);
  }
}


void autopilot() {
  lookForward();
  if (dist[med] > 40) {
    forward();
    delay(100);
  } else {
    stop();
    radar();
    backward();
    delay(100);
    stop();
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

void loop() {
  detectStop();
  if (runAutopilot) {
    autopilot();
  }
//  speedCorrection();
//  delay(maxSpeedDelay);
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    switch (inChar) {
      case 'a':
        radar();
        radar(true);
        lookForward();
        printAngleDist();
        break;
      case 'f':
        forward();
        break;
      case 'b':
        backward();
        break;
      case 'l':
        left();
        break;
      case 'r':
        right();
        break;
      case 's':
        speed = map(Serial.parseInt(), 0, 255, 0, 255);
        Serial.print("Set speed to ");
        Serial.println(speed);
        printSpeed();
        break;
      case 'q':
        runAutopilot = false;
        stop();
        break;
      case 'g':
        runAutopilot = true;
        break;
    }
    Serial.println(inChar);
  }
}



//void speedCorrection() {
//  if (speed0 > 0 && speed1 > 0) {
//    leftSpeed = speed1 / max(speed0, speed1) * 255.;
//    rightSpeed = speed0 / max(speed0, speed1) * 255.;
//    motor1.setSpeed(motor1Speed);
//    motor2.setSpeed(motor2Speed);
//  }
//}

void detectStop() {
  unsigned long m = millis();
  if (m - m0 > maxSpeedDelay) speed0 = 0;
  if (m - m1 > maxSpeedDelay) speed1 = 0;
}

void printSpeed() {
  Serial.print(speed0);
  Serial.print(" - ");
  Serial.print(speed1);
  Serial.println(" (m/s)");
}

void InitialiseIO(){
  pinMode(A0, INPUT);	   // Pin A0 is input to which a switch is connected
  digitalWrite(A0, HIGH);   // Configure internal pull-up resistor
  pinMode(A1, INPUT);	   // Pin A1 is input to which a switch is connected
  digitalWrite(A1, HIGH);   // Configure internal pull-up resistor
}

void InitialiseInterrupt(){
  cli();		// switch interrupts off while messing with their settings  
  PCICR =0x02;          // Enable PCINT1 interrupt
  PCMSK1 = 0b00000011;
  sei();		// turn interrupts back on
}

inline double filter(double p, double n) {
  double factor = 0.1;
  return p * (1.0 - factor) + n * factor;
}

ISR(PCINT1_vect) {    
  // Interrupt service routine. Every single PCINT8..14 (=ADC0..5) change
  // will generate an interrupt: but this will always be the same interrupt routine
  int currA0 = digitalRead(A0), currA1 = digitalRead(A1);
  unsigned long m = millis();
  if (currA0==HIGH && pinA0==LOW) {
    speed0 = filter(speed0, CD / (double)(m - m0) * 1000.);
    m0 = m;
  }
  if (currA1==HIGH && pinA1 == LOW) {
    speed1 = filter(speed1, CD / (double)(m - m1) * 1000.);
    m1 = m;
  }
  pinA0 = currA0; pinA1 = currA1;
}
