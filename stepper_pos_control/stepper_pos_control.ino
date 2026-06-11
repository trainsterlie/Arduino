#include <TMC2209.h>

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
const int32_t RUN_VELOCITY = 12000;
const int32_t STOP_VELOCITY = 0;
// current values may need to be reduced to prevent overheating depending on
// specific motor and power supply voltage
const uint8_t RUN_CURRENT_PERCENT = 300;
int press_instance = 0, steps_takne = 0, angle_new = 0, angle_old = 0;
int NUM_READ = 100, READINGS_ARR[100], current_idx = 0;
double angle_per_microstep = 0.9/256, f_constant = (12.5e6)/(pow(2,24));
// Instantiate TMC2209
TMC2209 stepper_driver;
bool full = false;
void setup()
{
  Serial.begin(115200);
  stepper_driver.setup(soft_serial);
  digitalWrite(EN_PIN, LOW);
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.setMicrostepsPerStepPowerOfTwo(8);
  stepper_driver.enableCoolStep();
  stepper_driver.enable();
}

unsigned int sum(int *array, int NUM){
  unsigned int total = 0;
  for (int i = 0;i<NUM;i++){
    total += array[i];
  }
  return total;
}


void loop()
{
  uint32_t angle = abs(map(analogRead(POT_PIN), 0, 1023, 0, 360));
  while (current_idx < NUM_READ && !full){
    READINGS_ARR[current_idx++] = angle;
    if (current_idx >= NUM_READ){
      full = true;
    }
  }
  if (full){
    if (current_idx >= NUM_READ){
      current_idx = 0;
    }
    READINGS_ARR[current_idx++] = angle;
  int angle_avg = sum(READINGS_ARR, NUM_READ)/NUM_READ;
  Serial.println(angle_avg);
  long int diff = angle_avg-angle_old;
  if (abs(diff) > 3){
    if (diff>0){
      stepper_driver.enableInverseMotorDirection();
      //Serial.println(((angle*1000)/(RUN_VELOCITY*angle_per_microstep)));
      stepper_driver.moveAtVelocity(RUN_VELOCITY);
      unsigned long now = millis();
      while ((double)(millis() - now) < ((diff*1000)/(RUN_VELOCITY*angle_per_microstep*f_constant))){
      }
      stepper_driver.moveAtVelocity(STOP_VELOCITY);
    }
    else{
      stepper_driver.disableInverseMotorDirection();
      //Serial.println(((angle*1000)/(RUN_VELOCITY*angle_per_microstep)));
      stepper_driver.moveAtVelocity(RUN_VELOCITY);
      unsigned long now = millis();
      while ((double)(millis() - now) <((abs(diff)*1000)/(RUN_VELOCITY*angle_per_microstep*f_constant))){
      }
      stepper_driver.moveAtVelocity(STOP_VELOCITY);
    }
    angle_old = angle_avg;
  }
  }
  else{
    stepper_driver.moveAtVelocity(STOP_VELOCITY);
  }
}
