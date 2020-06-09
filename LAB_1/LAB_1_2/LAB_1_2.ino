#include <TimerOne.h>

const int RLED_PIN = 12;
const int GLED_PIN = 11;

const float R_HALF_PERIOD = 1.5;
const float G_HALF_PERIOD = 3.5;

volatile int greenLedState = LOW;
volatile int redLedState = LOW;

void blinkGreen() {
  greenLedState = !greenLedState;
  digitalWrite(GLED_PIN, greenLedState);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Lab 1.2 Starting");
  pinMode(RLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);
  Timer1.initialize(G_HALF_PERIOD * 1e06);
  Timer1.attachInterrupt(blinkGreen);

}
void serialprintstatus(){
    if (Serial.available() > 0) {
      int inByte = Serial.read();
      Serial.print("I receive: ");
      if (inByte == (int)'G') {
        Serial.print("Green LED:");
        Serial.println(greenLedState);
    }
    else if (inByte == (int)'R') {
       Serial.print("Red LED:");
       Serial.println(redLedState);
    }
    else {
       Serial.println("Wrong input!");
    }
  }
}

void loop() {

  redLedState = !redLedState;
  digitalWrite(RLED_PIN, redLedState);     
  delay(R_HALF_PERIOD * 1e03);
  serialprintstatus();
}
