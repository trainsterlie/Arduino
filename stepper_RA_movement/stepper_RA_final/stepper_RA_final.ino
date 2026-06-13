//Final sketch of the star tracker
#include <TMC2209.h>
#include <SoftwareSerial.h>
#define STEP_ANGLE 0.9
#define RATIO 150 //this is the total ratio of the gearbox+pulley system. In our case a 3:1 pulley and a 50:1 harmonic gearbox results in a total 150:1 reduction
#define STEPS 8 //must be in 2^n form, eg 2,4,8,16,64,128,256
#define STEP_INTERVAL 50 //this is the delay between each pulse to the stepper motor to ensure it catches the step pwm
const uint8_t RX_PIN = 10;
const uint8_t TX_PIN = 11;
const uint8_t EN_PIN = 3;
const uint8_t STEP_PIN = 6;
const uint8_t DIR_PIN = 5;
unsigned long first, last;
unsigned long interval;
SoftwareSerial soft_serial(RX_PIN, TX_PIN);
const uint32_t time_between_pulses = (((86164.0905/360)*STEP_ANGLE*RATIO)/STEPS)*1000000UL; //in microseconds
const uint32_t RUN_VELOCITY =  int((((360/STEP_ANGLE)*STEPS*RATIO)/86164.0905)*1.398101)
TMC2209 stepper_driver;
void setup()
{
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW);
  digitalWrite(STEP_PIN, LOW);
  Serial.begin(9600);
  soft_serial.begin(9600);
  stepper_driver.setup(soft_serial, 9600);
  stepper_driver.setMicrostepsPerStep(STEPS);
  stepper_driver.enableAutomaticCurrentScaling();
  stepper_driver.enableAutomaticGradientAdaptation();
  stepper_driver.disableInverseMotorDirection();
  stepper_driver.setRMSCurrent(500, 0.22); 
  stepper_driver.enable();
  first = micros();
  
}
void Pulse(void){
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(STEP_INTERVAL);
  digitalWrite(STEP_PIN, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  if ((micros()-first) >= time_between_pulses){
    first += time_between_pulses
    Pulse();
  }
}
