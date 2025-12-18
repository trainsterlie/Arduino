#include <PWM.h>
/*
 * Pulse per revolution is 2 for the NOCTUA NF-AF12 PPC2000, PIN 6allocated for PWM output, PIN 3 allocated for the fan speed RPM PWM Signal
 * 
 * [CHANGES]
 * Prev concept:
 * - New instantaneous RPM calculated on every ISR callback
 * - Fills 100 entry array before iterating through & averaging
 * - Very frequent toggling of interrupts
 * - Large array size (100 doubles!)
 * - Slow response (arduinos are damn slow at float math, even worse for double cos no FPU)
 *
 * Suggested architecture:
 * - Sample over a specified time window (500ms), can be shortened for faster response
 * - Within the window, the ISR only increments a counter to count pulses
 * - When window closes (>500ms elapsed), disable interrupts (cli())
 * - Look at how many pulses were counted in this 500ms to get RPM (cumulative, no need to iterate through previous data)
 * - Re-enable interrupts (sei()) & reset counter
 * - Only one step requires floating point math (final RPM estimation)!
 */

/* Pin definitions */
constexpr uint8_t fan_pwm = 10;
constexpr uint8_t fan_tach = 3;
constexpr uint8_t pot_pin = A1;

/* Pulse counter var */
volatile uint32_t cnt = 0;
constexpr uint8_t ppr = 2;

/* Timestamp vars */
unsigned long pulse_tmstp = 0;  // micros!
unsigned long window_tmstp = 0; // millis!

/* RPM cache */
float rpm = 0;

/*
 * @brief ISR callback
 * @note Only count pulses >10ms (10000us) apart
 */
void ISR_pulseIn(){
  unsigned long curr = micros();
  if(curr - pulse_tmstp > 5000){
    cnt++;
    pulse_tmstp = curr;
  }
}
void setup() {
  /* Init fan pwm timer to 25kHz */
  InitTimersSafe();
  SetPinFrequencySafe(fan_pwm, 25000);
  pinMode(fan_pwm, OUTPUT); 
  pinMode(fan_tach, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(fan_tach), ISR_pulseIn, FALLING);
  Serial.begin(9600);
}

void loop() {
  /* Map pot in to fan pwm */
  int pwm = map(analogRead(pot_pin), 0, 1023, 0, 255);
  pwmWrite(fan_pwm, pwm);

  unsigned long now = millis();
  unsigned long elapsed = now - window_tmstp;

  /* Sampling window: 500ms (2hz) */
  if(elapsed > 500){
    /* Disable interrupts */
    cli();
    uint32_t cnt_cache = cnt;
    cnt = 0;

    /* Enable interrupts */
    sei();

    /* Compute rpm & print */
    rpm = (60000.0f * cnt_cache) / (ppr * elapsed) ;
    Serial.println(rpm, 3);
    window_tmstp = now;
  }
} 