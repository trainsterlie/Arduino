#include <TMC2209.h>
#include <SoftwareSerial.h>
#define RATIO 150
#define STEPS 256 //must be in 2^n form, eg 2,4,8,16,64,128,256
#define SLEW_SPEED 12000
// SoftwareSerial can be used on Arduino boards without HardwareSerial ports,
// such as the Uno, Nano, and Mini.
//
// See this reference for more details:
// https://www.arduino.cc/reference/en/language/functions/communication/serial/

// Software serial ports should only be used for unidirectional communication
// The RX pin does not need to be connected, but it must be specified when
// creating an instance of a SoftwareSerial object
const uint8_t RX_PIN = 10;
const uint8_t TX_PIN = 11;
const uint8_t EN_PIN = 3;
const uint8_t STEP_PIN = 6;
const uint8_t DIR_PIN = 5;
const int POT_PIN = A0;
const uint8_t BUT_REV_PIN = 9;
const uint8_t BUT_SLEW_PIN = 12;
SoftwareSerial soft_serial(RX_PIN, TX_PIN);
const int32_t RUN_VELOCITY = 249; //249

const int32_t STOP_VELOCITY = 0;
int pot_value;
bool inversed = true;
const double time_between_pulses = (((86164.0905/360)*0.9*RATIO)/STEPS)*1000000; //in microseconds
// current values may need to be reduced to prevent overheating depending on
// specific motor and power supply voltage

// Instantiate TMC2209
TMC2209 stepper_driver;
void setup()
{
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(BUT_REV_PIN, INPUT_PULLUP);
  pinMode(BUT_SLEW_PIN, INPUT_PULLUP);
  digitalWrite(DIR_PIN, LOW);
  digitalWrite(STEP_PIN, LOW);
  Serial.begin(9600);
  soft_serial.begin(9600);
  stepper_driver.setup(soft_serial, 9600);
  Serial.println(stepper_driver.isCommunicating());
  stepper_driver.setMicrostepsPerStep(STEPS);
  //stepper_driver.disableCoolStep();
  stepper_driver.enableAutomaticCurrentScaling();
  stepper_driver.enableAutomaticGradientAdaptation();
  stepper_driver.enableInverseMotorDirection();
  stepper_driver.setRMSCurrent(200, 0.11); 
  stepper_driver.enable();
  stepper_driver.moveAtVelocity(RUN_VELOCITY);
}

void Pulse(const uint8_t step){
  delayMicroseconds(time_between_pulses);
  digitalWrite(STEP_PIN, HIGH);
  digitalWrite(STEP_PIN, LOW);
}
bool ButtonPress(const uint8_t PIN){
  int but_value = digitalRead(PIN);
  bool pressed = false;
  if (but_value == LOW){
    pressed = true;
    Serial.println("Pressed");
    delay(500);
  }
  else{
    pressed = false;
  }
  return pressed;
}
void Slew(const uint32_t SLEW_PIN){
  int but_value = digitalRead(SLEW_PIN);
  stepper_driver.moveAtVelocity(SLEW_SPEED);
  Serial.println("Slewing!");
  while (digitalRead(SLEW_PIN) == LOW){}
  stepper_driver.moveAtVelocity(RUN_VELOCITY);
  Serial.println("Stopped Slewing");
}
void loop()
{
  if (ButtonPress(BUT_REV_PIN)){
    if (inversed){
      stepper_driver.disableInverseMotorDirection();
      inversed = false;
    }
    else{
      stepper_driver.enableInverseMotorDirection();
      inversed = true;
  }
  }
  if (ButtonPress(BUT_SLEW_PIN)){
    Slew(BUT_SLEW_PIN);
  }
  }
  //pot_value = map(analogRead(POT_PIN), 0, 1023, 0, 800);
  //Serial.println(pot_value);
  //stepper_driver.setRMSCurrent(pot_value, 0.22);
  //delay(100);
  //Pulse(STEP_PIN);
  //Serial.println(stepper_driver.isSetupAndCommunicating());
  

