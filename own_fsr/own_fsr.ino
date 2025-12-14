const int FSR_PIN = 0;
const int PULL_R = 10000;
const int V_IN = 5;
float weight;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  float analogV = (analogRead(FSR_PIN)/1024.0)*5;
  float FSR_R = ((V_IN/analogV)-1)*PULL_R;
  float gradient;
  Serial.println("Analog Voltage: " + String(analogV));
  Serial.println("Resistance: " + String(FSR_R));
  if (analogV > 0){
    //Serial.println("Resistance: " + String(FSR_R));
    if (FSR_R < 30000) {
      gradient = -2975/998;
      weight = (FSR_R-30000)/gradient;
    } 
    else if (FSR_R < 100000){
      gradient = 700000/-3;
      weight = (FSR_R-100000)/gradient;
    } 
    else{
      Serial.println("Too heavy! Cannot Measure!");
    }
    Serial.print("The weight is ");
    Serial.println(weight);
  }
  else{
    Serial.println("No pressure detected!");
  }
  delay(500);
}
