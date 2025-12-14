const int OUT_PIN = 3;
const int SCK_PIN = 5;
void setup() {
  // put your setup code here, to run once:
  pinMode(OUT_PIN, INPUT);
  pinMode(SCK_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (digitalRead(OUT_PIN)){
  }
  long result  =0;
  for (int i =0; i<24; i++){
    digitalWrite(SCK_PIN, HIGH);
    digitalWrite(SCK_PIN, LOW);
    result = result<<1;
    if (digitalRead(OUT_PIN)){
      result++;
    }
  }
  result = result ^ 0x800000;
  for (char i = 0; i<3;i++){
    digitalWrite(SCK_PIN, HIGH);
    digitalWrite(SCK_PIN, LOW);
  }
  Serial.println(result);
}
