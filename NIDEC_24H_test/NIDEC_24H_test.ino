const uint32_t EN_PIN = 5;
const uint32_t DIR_PIN = 6;
const uint32_t PWM_PIN = 9;
const uint32_t POT_PIN = A0;
uint32_t pot_value;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
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

}

void loop() {
  // put your main code here, to run repeatedly:
  pot_value = map(analogRead(POT_PIN), 0, 1023, 0, 510);
  delay(500);
  Serial.print("Current pot_value: ");
  Serial.println(pot_value);
  OCR1A = pot_value;
}
