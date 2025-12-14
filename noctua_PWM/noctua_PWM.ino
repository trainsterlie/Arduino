#include <math.h>
#include <time.h>
#include <stdbool.h>
const int PPR = 4,FAN_SIG = 6, int FAN_READ = 3; //Pulse per revolution is 4 for the NOCTUA NF-AF12 PPC2000, PIN 6allocated for PWM output, PIN 3 allocated for the fan speed RPM PWM Signal
int num_readings = 100,read_index=0, print_interval = 1000; //number of readings for RPM, read_idx and how long between each print
volatile int count, CurrentTime, LastTime, AskTime; //since these values are always changing, we assign volatile
unsigned long duration;
double total, average, RPM, readings[100];
bool exceeded = false, asking = true, done = false;;
String input;

void Pulse_Event(){
  CurrentTime = micros(); //checking for a valid pulse
  count++;
}
void setup() {
  // put your setup code here, to run once:
  pinMode(FAN_SIG, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(FAN_READ), Pulse_Event, RISING);
  Serial.begin(9600);
  Serial.println("Simply enter your desired RPM at any point...");
  delay(1000);
  AskTime = millis();
  LastTime = micros();
}

void loop() {
  // put your main code here, to run repeatedly
  if (count >= PPR && CurrentTime>LastTime){ //if we hit count == PPR, means that one revolution has passed. We also check that our duratin is valid.
    noInterrupts();
    duration = CurrentTime-LastTime; //this is the time taken for 1 revolution in microseconds
    RPM = 60E6/duration; //RPM calculation
    readings[read_index++] = RPM; //we add to the readings array the RPM reading for read_index
    if (read_index >= num_readings){ //if we exceeded max number of allocated space, we overflow to 0
      read_index = 0; 
      exceeded = true;
    }
    count = 0;
    done = true;
    LastTime = CurrentTime; //immediately assign lasttime to currenttime since attachInterrupt will always be listenign to the signal
    interrupts();
  }
  if (exceeded){ //make sure we initialised our entire readings array first before we average out the readings
    total = 0;
    for (int i =0;i<num_readings;i++){
      if (readings[i]){
        total += readings[i]; 
        }
    }
    average = total/num_readings;
    while (Serial.available() > 0){
      input = Serial.readStringUntil('\n');
      Serial.print("You have selected");
      Serial.print(input);
      Serial.println("RPM for the fan!");
      analogWrite(FAN_SIG, ((float)input.toInt()/2000)*255);
      }
    input = "";
    if (millis() - AskTime >= print_interval){
      Serial.print("Current RPM is ");
      Serial.print(average);
      Serial.print("RPM And airflow is approximately ");
      Serial.print(average*71.69/2000);
      Serial.print(" CFM");
    }
  }

} 
