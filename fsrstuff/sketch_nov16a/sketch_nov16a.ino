int fsrPin = 5;     // the FSR and 10K pulldown are connected to a0
int fsrReading;     // the analog reading from the FSR resistor divider
int fsrVoltage;     // the analog reading converted to voltage
unsigned long fsrResistance;  // The voltage converted to resistance
unsigned long fsrConductance; 
float fsrForce;       // Finally, the resistance converted to force
 
void setup(void) {
  Serial.begin(9600);   // We'll send debugging information via the Serial monitor
}
 
void loop(void) {
  fsrReading = analogRead(fsrPin);  
  Serial.print("Analog reading = ");
  Serial.println(fsrReading);
 
  // analog voltage reading ranges from about 0 to 1023 which maps to 0V to 5V (= 5000mV)
  fsrVoltage = map(fsrReading, 0, 1023, 0, 5000);
  Serial.print("Voltage reading in mV = ");
  Serial.println(fsrVoltage);  
 
  if (fsrVoltage == 0) {
    Serial.println("No pressure");  
  } else {
    // The voltage = Vcc * R / (R + FSR) where R = 10K and Vcc = 5V
    // so FSR = ((Vcc - V) * R) / V        yay math!
    fsrResistance = 5000 - fsrVoltage;     // fsrVoltage is in millivolts so 5V = 5000mV
    fsrResistance *= 9850;                // 10K resistor
    fsrResistance /= fsrVoltage;
    Serial.print("FSR resistance in ohms = ");
    Serial.println(fsrResistance);
 
    fsrConductance = 1000000;           // we measure in micromhos so 
    fsrConductance /= fsrResistance;
    Serial.print("Conductance in microMhos: ");
    Serial.println(fsrConductance);
 
    // Use the two FSR guide graphs to approximate the force
    if (fsrConductance <= 1000) {
    // Use 80.0 for float division
    fsrForce = (float)fsrConductance / 80.0; 
    Serial.print("Force in Newtons: ");
    Serial.println(fsrForce);      
    
    // Weight conversion: Force (N) / 9.81 * 1000
    Serial.print("Weight in grams: ");
    Serial.println(fsrForce / 9.81 * 1000.0);
    } else {
    // Use 30.0 for float division
    fsrForce = (float)(fsrConductance - 1000) / 30.0;
    Serial.print("Force in Newtons: ");
    Serial.println(fsrForce);           
    
    // Weight conversion: Force (N) / 9.81 * 1000
    Serial.print("Weight in grams: ");
    Serial.println(fsrForce / 9.81 * 1000.0);
}
  }
  Serial.println("--------------------");
  delay(1000);
}
