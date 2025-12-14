#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <Servo.h>
#include <stdlib.h>
String angleS;
Servo angle_servo;
const int SERVO_PIN = 5, START_ANGLE = 60, MAX_ANGLE; 
const int POT_PIN = 1, PPR = 4,FAN_SIG = 6,FAN_READ = 3; //Pulse per revolution is 4 for the NOCTUA NF-AF12 PPC2000, PIN 6allocated for PWM output, PIN 3 allocated for the fan speed RPM PWM Signal
int num_readings = 100,read_index=0, print_interval = 1000; //number of readings for RPM, read_idx and how long between each print
volatile int count, CurrentTime, LastTime, AskTime; //since these values are always changing, we assign volatile
unsigned long duration;
double total, average, RPM, readings[100];
bool exceeded = false, asking = true, done = false;;
String input;

void Pulse_Event(){
  CurrentTime = micros(); //checking for a valid pulse
  count++;
}
void setup() {
  // put your setup code here, to run once:
  angle_servo.attach(SERVO_PIN);
  angle_servo.write(START_ANGLE);
  pinMode(POT_PIN, INPUT);
  pinMode(FAN_SIG, OUTPUT); //set PWM output with pin FAN_SIG
  attachInterrupt(digitalPinToInterrupt(FAN_READ), Pulse_Event, RISING);
  Serial.begin(9600);
  Serial.println("Simply enter your desired RPM at any point...");
  delay(1000);
  AskTime = millis();
  LastTime = micros();
}

void loop() {
  // put your main code here, to run repeatedly
  if (count >= PPR && CurrentTime>LastTime){ //if we hit count == PPR, means that one revolution has passed. We also check that our duratin is valid.
    noInterrupts();
    duration = CurrentTime-LastTime; //this is the time taken for 1 revolution in microseconds
    RPM = 60E6/duration; //RPM calculation
    readings[read_index++] = RPM; //we add to the readings array the RPM reading for read_index
    if (read_index >= num_readings){ //if we exceeded max number of allocated space, we overflow to 0
      read_index = 0; 
      exceeded = true;
    }
    count = 0;
    done = true;
    LastTime = CurrentTime; //immediately assign lasttime to currenttime since attachInterrupt will always be listenign to the signal
    interrupts();
  }
  if (exceeded){ //make sure we initialised our entire readings array first before we average out the readings
    total = 0;
    for (int i =0;i<num_readings;i++){
      if (readings[i]){
        total += readings[i]; 
        }
    }
    average = total/num_readings;
    int val = map(analogRead(POT_PIN), 0, 1023, 0, 255); //here we map the analogRead min-max of 0-1023 to analogWrite range of 0-255
    analogWrite(FAN_SIG, val); //here we write the mapped value from the potentiometer to the PWM signal of the fan
    Serial.print("Enter your desired airfoil angle");
    while (Serial.available() > 0){  //get desired airfoil angle
      angleS = Serial.readStringUntil('\n');
      angleS.trim();
      int angle = angleS.toInt();
      if (angle >= -MAX_ANGLE && angle <= MAX_ANGLE){
        angle_servo.write(angle+90);
        Serial.print("Moving Servo to angle");
        Serial.println(angle);
        }
      else{
        Serial.println("Enter angle you want the airfoil to be at: ");
      }
    angleS = "";
  }
    if (millis() - AskTime >= print_interval){ //printing of RPM and CFM results
      Serial.print("Current RPM is ");
      Serial.print(average);
      Serial.print("RPM And airflow is approximately ");
      Serial.print(average*71.69/2000);
      Serial.print(" CFM");
    }
  }

} 
