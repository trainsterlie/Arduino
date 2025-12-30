#include <TMC2209.h>
#include <stdbool.h>
// SoftwareSerial can be used on Arduino boards without HardwareSerial ports,
// such as the Uno, Nano, and Mini.
//
// See this reference for more details:
// https://www.arduino.cc/reference/en/language/functions/communication/serial/

// Software serial ports should only be used for unidirectional communication
// The RX pin does not need to be connected, but it must be specified when
// creating an instance of a SoftwareSerial object
const uint8_t RX_PIN = 9;
const uint8_t TX_PIN = 10;
const uint8_t POT_PIN = A0, BUT_PIN = 11;
SoftwareSerial soft_serial(RX_PIN, TX_PIN);
bool BUT_PRESS;
int press_instant;
const int32_t HOLD_DUR = 2000;
const int32_t RUN_VELOCITY = 20000;
const int32_t STOP_VELOCITY = 0;
const int RUN_DURATION = 2000;
const int STOP_DURATION = 1000;
// current values may need to be reduced to prevent overheating depending on
// specific motor and power supply voltage
const uint8_t RUN_CURRENT_PERCENT = 100;


// Instantiate TMC2209
TMC2209 stepper_driver;
bool invert_direction = false;

void setup()
{
  Serial.begin(115200);
  stepper_driver.setup(soft_serial);
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.enableCoolStep();
  stepper_driver.enable();
  pinMode(BUT_PIN, INPUT);
}

void loop()
{
  if (not stepper_driver.isSetupAndCommunicating())
  {
    Serial.println("Stepper driver not setup and communicating!");
    return;
  }
  int step_velo = map(analogRead(POT_PIN), 0, 1023, 0, RUN_VELOCITY);
  if (digitalRead(BUT_PIN) == HIGH){
    BUT_PRESS = true;
    press_instant = millis();
  }
  elif (digitalRead(BUT_PIN) == LOW && BUT_PRESS){
    BUT_PRESS = false;
  }
  if (((millis() - press_instant )> 1000) && BUT_PRESS){
    stepper_driver.moveAtVelocity(step_velo);
  }
}