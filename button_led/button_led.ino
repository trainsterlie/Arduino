const int LED_PIN = 11;
const int BUTT = 3;
int buttonstate = 0;
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTT, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  buttonstate = digitalRead(BUTT);
  if (buttonstate == HIGH){
    digitalWrite(LED_PIN, HIGH);
    Serial.write("HIGH");
  } 
  else{
    digitalWrite(LED_PIN, LOW);
    Serial.write("LOW");

  }
}
