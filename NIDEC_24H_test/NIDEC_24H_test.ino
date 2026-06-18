#include <Arduino.h>
const uint8_t EN_PIN = 5;
const uint8_t DIR_PIN = 6;
const uint8_t PWM_PIN = 9;
const uint8_t POT_PIN = A0;
const uint8_t ENC_A = 2;
const uint8_t ENC_B = 3;
uint32_t pot_value;
uint8_t direction = 0;
uint32_t RPM;
volatile uint8_t counter;
#define A 0
#define B 1
#define CW 1
#define CCW 0
#define PPR 100
typedef struct container{
  uint8_t identifier;
  volatile uint8_t state = 0;
  volatile uint32_t timing = 0;
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
  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);
  
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
  attachInterrupt(digitalPinToInterrupt(ENC_A), PulseInterruptA, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_B), PulseInterruptB, RISING);
}

void PulseInterruptA(){
  FDBK_A.timing = millis()-FDBK_A.timing; //keep updating the timing every time the encoder passes
  if (FDBK_A.other->state){ //if the other signal has already arrived, means that the FDBK_A signal is the last one.
    if (FDBK_A.identifier == A) //if our current last wire is A, we judge that the shaft is moving CCW since B->A
      direction = CCW;
    else //else we judge that the shaft is moving CW
      direction = CW;
      !FDBK_A.other->state; //and we reset the other wire's state back to 0
  }
  else //else we just switch on the current wire's state
    !FDBK_A.state;
}
void PulseInterruptB(){
  FDBK_B.timing = millis()-FDBK_B.timing; //keep updating the timing every time the encoder passes
  if (FDBK_B.other->state){ //if the other signal has already arrived, means that the curr signal is the last one.
    if (FDBK_B.identifier == A) //if our current last wire is A, we judge that the shaft is moving CCW since B->A
      direction = CCW;
    else //else we judge that the shaft is moving CW
      direction = CW;
      !FDBK_B.other->state; //and we reset the other wire's state back to 0
  }
  else //else we just switch on the current wire's state
    !FDBK_B.state;
}
uint32_t calculate_rpm(volatile uint32_t time_between_pulses){
  return (time_between_pulses*PPR)/1000; //time taken for 1 revolution in milliseconds
}

void loop() {
  // put your main code here, to run repeatedly:
  pot_value = map(analogRead(POT_PIN), 0, 1023, 0, 510);
  delay(500);
  Serial.print("Current pot_value: ");
  Serial.println(pot_value);
  OCR1A = pot_value;
  Serial.print("RPM:");
  Serial.println(counter);
  RPM = uint32_t((calculate_rpm(FDBK_A.timing) + calculate_rpm(FDBK_B.timing))/2);
}
