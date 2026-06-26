//main reaction wheel sketch
//functions to implement
//Error (PID) (Done)
//Motor Drive Function(Motor Smoothing), takes in the target rpm and smoothens out the acceleration of the motor until it reaches the target rpm (Done)
//Brake Function (enable the Brake pin) (Done)
//Reverse Function (enables/disables the Direction pin) (Done)
#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>
const uint8_t EN_PIN = 5;
const uint8_t DIR_PIN = 6;
const uint8_t PWM_PIN = 9;
const uint8_t BRAKE_PIN = 7;
const uint8_t POT_PIN = A0;
const uint8_t ENC_A = 2;
const uint8_t ENC_B = 3;
uint8_t past_A_state, commanded_direction = 0, direction = 0; //1 is CCW, 0 is clockwise
volatile uint8_t counter;
volatile uint32_t current_time;
double RPM, error, past_error, new_output, pot_value, cumulative_integral_val = 0;
double current_percent_level = 0;
uint32_t time_elapsed = 0, time_interval;
float accelX, accelY, accelZ;
double current_angle;
#define A 0
#define B 1
#define CW 0
#define CCW 1
#define PPR 100
#define EMA_MULTI 0.90
#define KP 0.01f //motor_pid values are 0.01, 0.006, 0.001
#define KI 0.006f
#define KD 0.001f
#define IMU_ADDRESS 0x68
//#define PERFORM_CALIBRATION 1
MPU6050 IMU;
calData calib = {0};
AccelData accelData;
GyroData gyroData;
typedef struct container{
  uint8_t identifier;
  volatile uint8_t state;
  volatile uint32_t timing = 0;
  volatile uint32_t elapsed_time = 0;
  container* other;
};
struct container FDBK_A;
struct container FDBK_B;
void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Wire.setClock(400000);
  Serial.begin(115200);
  while (!Serial){ //we wait for Serial to give us a signal indiciating we have data streaming through the i2c
    ; 
  }
  int err = IMU.init(calib, IMU_ADDRESS);
  #ifdef PERFORM_CALIBRATION //start of our calibration routine
    Serial.println("Calibrating Accelerometer...");
    delay(2000);
    Serial.println("Keep IMU in the neutral position.");
    delay(5000);
    IMU.calibrateAccelGyro(&calib);
    Serial.println("Accel biases X/Y/Z: ");
    Serial.print(calib.accelBias[0]);
    Serial.print(", ");
    Serial.print(calib.accelBias[1]);
    Serial.print(", ");
    Serial.println(calib.accelBias[2]);
    delay(5000);
    IMU.init(calib, IMU_ADDRESS);
  #endif
  //calib.accelBias[0] = 0.03;
  //calib.accelBias[1] = 0.03;
  //calib.accelBias[2] = 0.04;
  err = IMU.setGyroRange(500);
  err = IMU.setAccelRange(2);
  if (err != 0) {
  Serial.print("Error setting range: ");
  Serial.println(err);
  while (true){
    ;
  }
  }
  FDBK_A.identifier = A;
  FDBK_B.identifier = B;
  FDBK_A.other = &FDBK_B;
  FDBK_B.other = &FDBK_A;
  FDBK_A.state = digitalRead(ENC_A); //change here
  FDBK_B.state = digitalRead(ENC_B);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(BRAKE_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  // Clear Timer1 Control Registers
  TCCR1A = 0;
  // Set Timer1 to Fast PWM mode using ICR1 (Mode 14)
  // Enable PWM on Channel A (Pin 9) and Channel B (Pin 10)
  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
  // Set the frequency: 16MHz / (Prescaler * Desired_Freq)
  // 16,000,000 / (1 * 20,000) = 800. Subtract 1 because counting starts at 0.
  ICR1 = 799; 
  // Set Duty Cycle for Pin 9 (between 0 and 799)
  OCR1A = 400; // 50% duty cycle
  digitalWrite(BRAKE_PIN, HIGH); //sending HIGH to our BRAKE_PIN disables the brake
  digitalWrite(DIR_PIN, commanded_direction); //set our direction first here to the DIR_PIN, we start off with CCW
  attachInterrupt(digitalPinToInterrupt(ENC_A), PulseInterruptA, CHANGE); //attach interrupts to both encoder feedback wires
  attachInterrupt(digitalPinToInterrupt(ENC_B), PulseInterruptB, CHANGE);
  FDBK_A.elapsed_time = millis();
  FDBK_B.elapsed_time = millis();
}
void PulseInterruptA(){ //this time using CHANGE instead of just RISING or FALLING, so everytime this ISR is triggered we get the time between each pulse
  current_time = micros();
  FDBK_A.timing = current_time-FDBK_A.elapsed_time;//this is the time interval between each CHANGE pulse
  FDBK_A.elapsed_time = current_time; //and the total time taken so far in micros()
  past_A_state = FDBK_A.state; //we get the past state. 
  FDBK_A.state = !FDBK_A.state; //we get the current_state
  if (!past_A_state){ //the case where past state == 0, which means that the current state is RISING
    if (FDBK_A.other->state){ //if at this current state B is HIGH, it means that the direction is in CCW
      direction=CCW;
    }
    else{ //else if B's state is LOW at this point, we are moving in CW
      direction = CW;
    }
  }
}
void PulseInterruptB(){
  current_time = micros();
  FDBK_B.timing = current_time-FDBK_B.elapsed_time;
  FDBK_B.elapsed_time = current_time;
  FDBK_B.state = !FDBK_B.state; //we get the current_state
}
uint8_t get_direction(struct container A_Wire){ //simply returns direction of the motor, CW is 0 and CCW is 1
  return direction;
}
uint32_t calculate_rpm(volatile uint32_t time_between_pulses){  //time_between_pulses is in microseconds
  if (!time_between_pulses) //if time between pulses is 0, we return a rpm of 0
    return 0;
  return uint32_t(600000/(time_between_pulses)); //time taken for 1 revolution in microseconds
}
double expo_moving_average(uint32_t prev, uint32_t current, float multiplier){ //using the formula for exponential moving average to smoothen out the values
  return double(multiplier*current + (prev*(1-multiplier))); //large multiplier value to smoothen out more (more lag)
}
void check_validity_timing(void){ //this helper function is to check if rpm has gone to 0 (eg sudden stop)
  if (((micros()-FDBK_A.elapsed_time) > 5000) && ((micros()-FDBK_B.elapsed_time) > 5000)){ //since if the current time micros() has exceeded the last recorded time a pulse was detected by more than 1 seconds we reset the timing attribute
    FDBK_A.timing = 0;
    FDBK_B.timing = 0;
  }
}
double get_average_rpm(void){ //function here to get the current RPM through using exponenential moving average to smoothen out the RPM readings. You can use a higher EMA_MULTIPLIER value to smoothen out the readings even more at the cost of reaction time
  double new_rpm = expo_moving_average((calculate_rpm(FDBK_A.timing*2) + calculate_rpm(FDBK_B.timing*2))/2, RPM, EMA_MULTI); //we get the average of RPM readings from both A and B encoder values and consequently use the expo_moving_average formula to smoothen out the rpm values
  if (new_rpm > 9999) //occasionally at the start of the program the rpm will exceed a very high value, to avoid this we tentatively include a impossible placeholder 
    return RPM;
  RPM = new_rpm;
  return new_rpm;
}
double get_signed_rpm(void){
  double averaged = get_average_rpm();
  if (commanded_direction){
    return -1*averaged;
  }
  else{
    return averaged;
  }
}
void Brake(uint8_t BRAKE_PIN){ //implement the braking function of the motor
  digitalWrite(BRAKE_PIN, LOW); //setting the brake_pin to high triggers the brake function of the motor
  //delay(100); //implement delays here to ensure the motor brakes properly
  //check_validity_timing();
  OCR1A = 510;
  digitalWrite(BRAKE_PIN, HIGH);
  delay(50); //then another delay to ensure we have some time
}
void Change_Direction(uint8_t DIR_PIN){ //change direction of the motor
  Brake(BRAKE_PIN); //trigger the brake_pin to safely change directions
  commanded_direction = !commanded_direction;
  digitalWrite(DIR_PIN, commanded_direction); //make sure to change this to the correct movement
  //delay(50); //delay to ensure safety
}
void set_PWM(double percent_difference){
  Serial.print("current percent level: ");
  Serial.println(current_percent_level);
  current_percent_level += percent_difference;
  current_percent_level = constrain(current_percent_level, -100, 100);
  if (current_percent_level < 0 && commanded_direction != CCW){
    Change_Direction(DIR_PIN);
  }
  else if (current_percent_level > 0 && commanded_direction != CW){
    Change_Direction(DIR_PIN);
  }
  OCR1A = uint32_t(map(abs(current_percent_level), 0, 100, 510, 0)); //we set the percent speed here with respect to our identified duty values for 0 and 100% power
}
void PID(double (*CURRENT)(void), double TARGET, float Kp, float Ki, float Kd, void (*func)(double)){
  double current_val = CURRENT();
  /*if (commanded_direction == CCW){ //in replacement of this we could just make RPM positive to mark clockwise and negative to mark counter clockwise
    current_val = current_val*-1.0;
  }*/
  error = TARGET - current_val;
  //Serial.print("Error: ");
  //Serial.println(error);
  time_interval = micros()-time_elapsed;  
  time_elapsed = micros();
  cumulative_integral_val += ((time_interval/1000000.0)*error);
  new_output = Kp*(float(error)) + Ki*(cumulative_integral_val) + Kd*((error-past_error)/(time_interval/1000000.0));//core function here
  past_error = error;
  Serial.print("new_output");
  Serial.println(new_output);
  func(constrain(new_output, -100.0, 100.0)); //relies on linear scaling of rpm to duty cycle, where 0 duty cycle = maxrpm and 510 duty cycle = 0 rpm
}
//start of our accel functions
double get_angle(float X){
  X = double(constrain(X, -1.0, 1.0));
  if (abs(X)==1.0){
    return 0;
  }
  double angle = asin(X/1)*(60.0/2*M_PI); //cos inverse Z to get the current angle with respect to the vertical
  angle = EMA_MULTI*angle + (current_angle*(1-EMA_MULTI));
  return angle;
}
void set_ANGLE(double output_error){
  Motor_PID(output_error);
}
bool is_on_side(double angle){
  if (abs(angle) > 30){
    return true;
  }
  return false;
}
void Jump(double angle){
  if (angle>0){
    while (get_average_rpm() < 390){
      Motor_PID(400);} //blocking PID function here to ensure the motor gets up to fast enough speed to jerk itself upwards
  }
  else{
    while (get_average_rpm() <390){
      Motor_PID(-400);}
  }
  Brake(BRAKE_PIN);
  delay(100);
}

void Motor_PID(double TARGET){
  PID(get_signed_rpm, TARGET, KP, KI, KD, set_PWM); //sending a function pointer into PID
}
void Accel_PID(double TARGET){
  PID(get_angle, 0, KP, KI, KD, set_PWM);
}
void loop() {
  // put your main code here, to run repeatedly:
  IMU.update();
  IMU.getAccel(&accelData);
  accelX = accelData.accelX;
  accelY = accelData.accelY;
  accelZ = accelData.accelZ;
  Serial.print(accelData.accelX);
  Serial.print("\t");
  Serial.print(accelData.accelY);
  Serial.print("\t");
  Serial.print(accelData.accelZ);
  Serial.print("\t");
  current_angle = get_angle(accelX);
  Serial.println(current_angle);
  if (Serial.available() > 0){
    char charac = Serial.read();
    if (charac == 'B'){
      Serial.println("Braking");
      Brake(BRAKE_PIN);
    }
    else if (charac == 'R'){
      Serial.println("Reversing");
      Change_Direction(DIR_PIN);
    }
  }
  pot_value = map(analogRead(POT_PIN), 0, 1023, -2000, 2000); //original rightmost value is 510
  Serial.print("Requested RPM: ");
  Serial.println(pot_value);
  //Serial.print(",");
  Serial.print("Current RPM: ");
  Serial.println(get_signed_rpm());
  delay(50);
  //Serial.print("Current pot_value: ");
  //Serial.println(pot_value);
  Motor_PID(pot_value);
  //Accel_PID(0);
  //Serial.print("Direction: ");
  //.println(direction);
  check_validity_timing();
}
