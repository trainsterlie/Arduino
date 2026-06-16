#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <Servo.h>
#include <stdlib.h>
String angleS;
Servo angle_servo;
const int SERVO_PIN = 5, START_ANGLE = 60, MAX_ANGLE, POT_PIN =A1, max_reading = 200; 
static int val, old_val;
bool done = false;
String input;
void setup() {
  // put your setup code here, to run once:
  angle_servo.attach(SERVO_PIN);
  angle_servo.write(START_ANGLE);
  Serial.begin(9600);
  Serial.print("Turn the potentiometer to get your desired angle!");
  old_val = START_ANGLE;
}

void loop() {
  // put your main code here, to run repeatedly:
  int total = 0, count = 0;
    while (count < max_reading){
      val = map(analogRead(POT_PIN), 0, 1023, 60, 120);
      total += val;
      count++;
    }
    val = total/(max_reading);
    if (abs(val-old_val) >= 1){
      angle_servo.write(val);
      Serial.print("Servo angle: ");
      Serial.println(val-90);
      old_val = val;
    }
}