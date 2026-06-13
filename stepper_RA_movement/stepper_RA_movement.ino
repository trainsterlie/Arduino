  #include <TMC2209.h>
  #include <stdbool.h>
  #include <SoftwareSerial.h>
  #define STEPS 256
  #define RATIO 35
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
  const uint8_t DIR_PIN = 3;
  const uint8_t STEP_PIN = 4;
  SoftwareSerial soft_serial(RX_PIN, TX_PIN);
  const uint8_t BUTTON_IN = 11;
  const double time_between_pulses = (((86164.0905/360)*0.9*RATIO)/STEPS)*1000000; //in microseconds
  // current values may need to be reduced to prevent overheating depending on
  // specific motor and power supply voltage
  const uint8_t RUN_CURRENT_PERCENT = 100;
  int pulses;
  unsigned int last_pulse, exceeded = 0;
  bool started_moving = false, running = false, last_button_state = false;
  // Instantiate TMC2209
  TMC2209 stepper_driver;
  void setup()
  {
  Serial.begin(9600);
  soft_serial.being(9600)
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  stepper_driver.setup(soft_serial, 9600);
  stepper_driver.setRMSCurrent(500, 0.22);
  stepper_driver.setMicrostepsPerStepPowerOfTwo(8);
  stepper_driver.enableAutomaticCurrentScaling();
  //stepper_driver.enableAutomaticGradientAdaptation();
  //stepper_driver.disableStealthChop();
  //stepper_driver.enableCoolStep();
  stepper_driver.setHoldCurrent(50);
  stepper_driver.setStandstillMode(stepper_driver.BRAKING);
  stepper_driver.enable();
  digitalWrite(DIR_PIN, HIGH);
  last_pulse = micros();
  }

  void pulseOut(uint8_t STEP_PIN){
    digitalWrite(STEP_PIN, HIGH);
    digitalWrite(STEP_PIN, LOW);
  }

  bool button_pressed(const uint8_t button){
    int now = micros();
    if (digitalRead(button)){
      delay(80);
      if (digitalRead(button)){
        return true;
      }
      else{
        return false;
      }
    }
    else{
      return false;
    }
  }

  void loop()
  {
    bool button_state_now = button_pressed(BUTTON_IN);
    if (button_state_now != last_button_state){
      running = !running;
      last_button_state = button_state_now;
      Serial.println("Boom");
      }
    if ((micros() + exceeded >= time_between_pulses) && running){
      exceeded = micros()-last_pulse;
      pulseOut(STEP_PIN); //pulse is done 
      Serial.println("Done");
      last_pulse = micros();
      if (!started_moving){
        started_moving = true;
      }
      pulses++;
    }
    //Serial.print("We have pulsed");
    //Serial.println(pulses );
  }