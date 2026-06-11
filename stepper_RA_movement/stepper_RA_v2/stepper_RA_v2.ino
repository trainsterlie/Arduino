#include <TMC2209.h>
#include <SoftwareSerial.h>
#define RATIO 150
#define STEPS 8 //must be in 2^n form, eg 2,4,8,16,64,128,256
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
const uint8_t BUT_PIN = 9;
SoftwareSerial soft_serial(RX_PIN, TX_PIN);
const int32_t RUN_VELOCITY = 1000;
const int32_t STOP_VELOCITY = 0;
int pot_value;
int but_value;
bool pressed;
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
  pinMode(BUT_PIN, INPUT_PULLUP);
  digitalWrite(DIR_PIN, LOW);
  digitalWrite(STEP_PIN, LOW);
  Serial.begin(9600);
  soft_serial.begin(9600);
  stepper_driver.setup(soft_serial, 9600);
  Serial.println(stepper_driver.isCommunicating());
  stepper_driver.setMicrostepsPerStep(STEPS);
  stepper_driver.enableAutomaticCurrentScaling();
  stepper_driver.enableAutomaticGradientAdaptation();
  stepper_driver.enableInverseMotorDirection();
  //stepper_driver.setRMSCurrent(400, 0.22); 
  stepper_driver.enable();
  stepper_driver.moveAtVelocity(RUN_VELOCITY);
}

void Pulse(const uint8_t step){
  delayMicroseconds(time_between_pulses);
  digitalWrite(STEP_PIN, HIGH);
  digitalWrite(STEP_PIN, LOW);
}
void loop()
{
  but_value = digitalRead(BUT_PIN);
  if (but_value == LOW){
    pressed = true;
    Serial.println("Pressed");
    if (inversed){
    stepper_driver.disableInverseMotorDirection();
    inversed = false;
    }
    else{
      stepper_driver.enableInverseMotorDirection();
      inversed = true;
    }
    delay(200);
  }
  else{
    pressed = false;
  }
  pot_value = map(analogRead(POT_PIN), 0, 1023, 0, 800);
  Serial.println(pot_value);
  stepper_driver.setRMSCurrent(pot_value, 0.22);
  //Pulse(STEP_PIN);
  //Serial.println(stepper_driver.isSetupAndCommunicating());
  

}