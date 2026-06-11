#include <TMC2209.h>
#include <SoftwareSerial.h>
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
const uint8_t EN_PIN = 6;
SoftwareSerial soft_serial(RX_PIN, TX_PIN);
const uint8_t POT_PIN = A0, BUTTON_IN = 11;
const int32_t RUN_VELOCITY = 8000;
const int32_t STOP_VELOCITY = 0;
const int RUN_DURATION = 2000;
const int STOP_DURATION = 1000;
// current values may need to be reduced to prevent overheating depending on
// specific motor and power supply voltage
const uint8_t RUN_CURRENT_PERCENT = 100;
int press_instance = 0;
// Instantiate TMC2209
TMC2209 stepper_driver;
bool invert_direction = false, failed = false;
bool moving = false;
void setup()
{
  Serial.begin(115200);
  stepper_driver.setup(soft_serial);
  digitalWrite(EN_PIN, LOW);
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.setMicrostepsPerStepPowerOfTwo(8);
  stepper_driver.enableCoolStep();
  stepper_driver.enableAutomaticCurrentScaling();
  stepper_driver.enableStealthChop();
  stepper_driver.enable();  
  
}

void loop()
{
  if (stepper_driver.isSetupAndCommunicating()){
    Serial.println("Setup and Communicating");
  }
  else{
    Serial.println("FAILURE");
  }
  stepper_driver.enableInverseMotorDirection();
  Serial.print("Moving at ");
  Serial.println(speed);
  stepper_driver.moveAtVelocity(RUN_VELOCITY, )
  }
  

}