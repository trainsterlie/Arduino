#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <PWM.h>
const int PPR = 2 ,FAN_SIG = 10, FAN_READ = 3, POT_PIN = A1; //Pulse per revolution is 2 for the NOCTUA NF-AF12 PPC2000, PIN 6allocated for PWM output, PIN 3 allocated for the fan speed RPM PWM Signal
int num_readings = 100,read_index=0, print_interval = 1000; //number of readings for RPM, read_idx and how long between each print
volatile unsigned long FirstPulse= 0, LastPulse = 0, AskTime=0, duration = 0; //since these values are always changing, we assign volatile
unsigned long TIMEOUT = 1000000UL;
double total, average, RPM, readings[100];
bool exceeded = false, asking = true, done = false, timeout = false;
String input;

void Pulse_Event(){
  unsigned long now = micros();
  FirstPulse = now;
  if (FirstPulse - LastPulse > 10000){
    duration = FirstPulse-LastPulse; //time taken for one pulse
    done = true;
  }
  LastPulse = FirstPulse;
}
void setup() {
    InitTimersSafe();
    bool success = SetPinFrequencySafe(FAN_SIG, 25000);

    pinMode(FAN_SIG, OUTPUT); 
    
  for (int i =0;i<num_readings;i++){
    readings[i] = 0;
    //clear the array
  }
  pinMode(FAN_READ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FAN_READ), Pulse_Event, FALLING);
  Serial.begin(9600);
  delay(1000);
  AskTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly
  int val = map(analogRead(POT_PIN), 0, 1023, 0, 255); //here we map the analogRead min-max of 0-1023 to analogWrite range of 0-255 //here we write the mapped value from the potentiometer to the PWM signal of the fan
  pwmWrite(FAN_SIG, val);
  Serial.println(duration);
  if (micros() - LastPulse > TIMEOUT && !timeout){
    for (int u =0; u<num_readings;u++){
      readings[u] = 0;
    }
    done=false;
    timeout = true;
  }
  if (done){ //wait for ISR to update value
    noInterrupts();
    RPM = (60e6/duration)/PPR; // since one pulse takes duration
    interrupts();
    if (read_index >= num_readings){
      read_index = 0; //reset back to 0 in the case of overflow
      exceeded = true;
    }
    readings[(read_index)++] = RPM; //write our RPM into the readings array
    done = false;
    timeout = false;

  }
  
  if (exceeded){ //make sure we initialised our entire readings array first before we average out the readings
    total = 0;
    for (int i =0;i<num_readings;i++){
      if (readings[i]){
        total += readings[i]; 
        }
    }
    average = total/num_readings;
    if (millis() - AskTime >= print_interval){

      Serial.print("Current RPM is ");
      Serial.print(average);
      Serial.print("RPM And airflow is approximately ");
      Serial.print(average*71.69/2000);
      Serial.println(" CFM");
      AskTime = millis();
    }
  }

} 
