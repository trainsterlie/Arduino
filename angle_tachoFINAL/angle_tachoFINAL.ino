#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <PWM.h>
#include <Servo.h>
const int PPR = 2 ,FAN_SIG = 10, FAN_READ = 3, POT_PIN = A1; //Pulse per revolution is 2 for the NOCTUA NF-AF12 PPC2000, PIN 6allocated for PWM output, PIN 3 allocated for the fan speed RPM PWM Signal
int num_readings = 100,read_index=0, print_interval = 1000; //number of readings for RPM, read_idx and how long between each print
volatile unsigned long FirstPulse= 0, LastPulse = 0, AskTime=0, duration = 0; //since these values are always changing, we assign volatile
unsigned long TIMEOUT = 1000000UL;
double total=0, average, RPM, readings[100];
bool exceeded = false, asking = true, done = false, timeout = false;
String angleS;
Servo angle_servo;
const int SERVO_PIN = 5, START_ANGLE = 60, MAX_ANGLE, max_reading = 100; 
static int val, old_val;

void Pulse_Event(){ //Here we define Pulse_event. This event(function) is triggered whenever FAN_READ receives a signal and it is FALLING. In our fan case this means falling from Vhigh to Vlow of the tach signal
  unsigned long now = micros(); //here we define current time in micros as now
  FirstPulse = now; 
  if (FirstPulse - LastPulse > 10000){ //since our duration will always be above a certain number (in this case we define 10000 micros) we filter out unwanted signals that are outside of this zone
    duration = FirstPulse-LastPulse; //time taken for one pulse
    done = true; //flag done as true here to indicate to our main loop that there is a new value of duration to update a new value of RPM. This is in case our main loop updates before our ISR does(unlikely since freq of tach is q high)
  }
  LastPulse = FirstPulse; 
}
void setup() {
  InitTimersSafe(); //part of PWM library idk wtf it does
  if (!SetPinFrequencySafe(FAN_SIG, 25000)){ //this function SetPinFrequencySafe returns a bool value if it has succesfully initialised or not
    return 0;
  }; //
  pwmWrite(FAN_SIG, 255);
  pinMode(FAN_SIG, OUTPUT); //set FAN_SIG as pwm output
  for (int i =0;i<num_readings;i++){ 
    readings[i] = 0;
    //clear the array
  }
  pinMode(FAN_READ, INPUT_PULLUP); //set pinmode of FAN_READ, tach pwm input
  attachInterrupt(digitalPinToInterrupt(FAN_READ), Pulse_Event, FALLING); //here we define our Interrupt routine, basically pulseIn on steroids
  angle_servo.attach(SERVO_PIN);
  angle_servo.write(START_ANGLE);
  Serial.begin(9600);
  Serial.print("Turn the potentiometer to get your desired angle!");
  old_val = START_ANGLE;
  delay(5000);
  AskTime = millis(); 
}


void loop() {
  // put your main code here, to run repeatedly
  int tots = 0, count = 0;
    while (count < max_reading){
      val = map(analogRead(POT_PIN), 0, 1023, 60, 120);
      tots += val;
      count++;
    }
    val = tots/(max_reading);
    if (abs(val-old_val) >= 1){
      angle_servo.write(val);
      Serial.print("Servo angle: ");
      Serial.println(val-90);
      old_val = val;
    }


  //Serial.println(duration); //part of debug where we print out the duration
  if (done){ //wait for ISR to update value
    noInterrupts();
    RPM = (60e6/duration)/PPR; // since one pulse takes duration
    interrupts();
    if (read_index >= num_readings){
      read_index = 0; //reset back to 0 in the case of overflow
      exceeded = true;
    }
    readings[read_index++] = RPM; //write our RPM into the readings array
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
    if (millis() - AskTime >= print_interval){
      average = total/num_readings;
      Serial.print("Current RPM is ");
      Serial.print(average);
      Serial.print("RPM And airflow is approximately ");
      Serial.print(average*71.69/2000);
      Serial.println(" CFM");
      AskTime = millis();
    }
  }

} 
