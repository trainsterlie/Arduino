#include <Arduino.h>
const uint8_t EN_PIN = 5;
const uint8_t DIR_PIN = 6;
const uint8_t PWM_PIN = 9;
const uint8_t POT_PIN = A0;
const uint8_t ENC_A = 2;
const uint8_t ENC_B = 3;
uint32_t pot_value;
uint8_t direction = 0;
uint32_t RPM = 0;
volatile uint8_t counter;
volatile uint32_t current_time;
#define A 0
#define B 1
#define CW 1
#define CCW 0
#define PPR 100
#define EMA_MULTI = 0.18
typedef struct container{
  uint8_t identifier;
  volatile uint8_t state = 0;
  volatile uint32_t timing = 0;
  volatile uint32_t elapsed_time = 0;
  container* other;
};
struct container FDBK_A;
struct container FDBK_B;
void setup() {
  // put your setup code here, to run once:
  FDBK_A.identifier = A;
  FDBK_B.identifier = B;
  FDBK_A.other = &FDBK_B;
  FDBK_B.other = &FDBK_A;
  Serial.begin(9600);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
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
  digitalWrite(EN_PIN, HIGH);
  digitalWrite(DIR_PIN, LOW);
  attachInterrupt(digitalPinToInterrupt(ENC_A), PulseInterruptA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), PulseInterruptB, CHANGE);
  FDBK_A.elapsed_time = millis();
  FDBK_B.elapsed_time = millis();
}
void PulseInterruptA(){ //this time using CHANGE instead of just RISING or FALLING, so everytime this ISR is triggered we get the time between each pulse
    current_time = micros()
    FDBK_A.timing = current_time-FDBK_A.elapsed_time;
    FDBK_A.elapsed_time = current_time;
  if (FDBK_A.other->state){ //if the other signal has already arrived, means that the FDBK_A signal is the last one, and the FDBK_B signal came first
      direction = CCW;
      FDBK_A.state = 0;
      FDBK_A.other->state = !FDBK_A.other->state; //and we reset the other wire's state back to 0
  }
  else{ //else we just switch on the current wire's state
    FDBK_A.state = !FDBK_A.state;}
}
void PulseInterruptB(){
  current_time = micros()
  FDBK_B.timing = current_time-FDBK_B.elapsed_time;
  FDBK_B.elapsed_time = current_time;
  if (FDBK_B.other->state){ //if the other signal has already arrived, means that the FDBK_A signal is the last one, and the FDBK_B signal came first
      direction = CW;
      FDBK_B.state = 0;
        FDBK_B.other->state = !FDBK_B.other->state; //and we reset the other wire's state back to 0
  }
  else{ //else we just switch on the current wire's state
    FDBK_B.state = !FDBK_B.state;}
}
uint32_t calculate_rpm(volatile uint32_t time_between_pulses){  
  if (!time_between_pulses){
    return 0;
  }
  return uint32_t(600000/(time_between_pulses)); //time taken for 1 revolution in milliseconds
}
uint32_t expo_moving_average(uint32_t prev_rpm, uint32_t current_rpm, float multiplier){
  return uint32_t(multiplier*current_rpm + (prev_rpm*(1-multiplier)))
}
void check_validity_timing(void){ //this helper function is to check if rpm has gone to 0 (eg sudden stop)
  if (((micros()-FDBK_A.elapsed_time) > 5000) && ((micros()-FDBK_B.elapsed_time) > 5000)){ //since if the current time millis() has exceeded the last recorded time a pulse was detected by more than 1 seconds we reset the timing attribute
    FDBK_A.timing = 0;
    FDBK_B.timing = 0;
  }
}
void loop() {
  // put your main code here, to run repeatedly:
  pot_value = map(analogRead(POT_PIN), 0, 1023, 0, 510);
  delay(50);
  //Serial.print("Current pot_value: ");
  //Serial.println(pot_value);
  OCR1A = pot_value;
  //Serial.print("RPM:");
  Serial.println(RPM);
  //Serial.print("Direction: ");
  //Serial.println(direction);
  check_validity_timing();
  RPM = expo_moving_average((calculate_rpm(FDBK_A.timing*2) + calculate_rpm(FDBK_B.timing*2))/2, RPM, EMA_MULTI);
}
