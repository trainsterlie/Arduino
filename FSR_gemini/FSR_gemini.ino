// FSR Power Law Calibration Sketch
// This sketch uses a two-point calibration to determine the constants
// (k and gamma) for the FSR's power-law curve: R_kOhm = k * F_g^-gamma

const int FSR_PIN = 5;
const float PULL_R = 9710.0; // 10k pull-down resistor value in Ohms
const float V_IN = 5.07;       // Arduino operating voltage in Volts

// --- Calibration Variables ---
float R1_kOhm = 0.0; 
float R2_kOhm = 0.0; 
float F1_grams = 0.0; 
float F2_grams = 0.0; 

// --- Calculated Power Law Constants ---
float FSR_POWER_K = 0.0;      // The constant 'k'
float FSR_GAMMA_EXPONENT = 0.0; // The '1/gamma' exponent
float TARE_OFFSET_GRAMS = 0.0; // Variable for zeroing the scale

bool is_calibrated = false;

// --- Filter Parameters ---
const int AVG_WINDOW_SIZE = 10; // Number of readings to average for smoother output
float readings[AVG_WINDOW_SIZE]; // Array to hold the last N weight readings
int readingIndex = 0;           // Current position in the array

// --- Function Prototypes ---
float calculateResistance_kOhm();
float calculateWeight_grams(float fsrResistance_kOhm);
void performCalibration();
void tareScale();


void setup() {
  Serial.begin(9600);
  Serial.println("--- FSR Calibration Starting ---");
  Serial.println("Please enter the weight (in grams) of your light calibration object.");
  Serial.println("Example: 50.0");
  
  // Wait for the user to input the light weight
  while (Serial.available() == 0) {
    delay(10);
  }
  F1_grams = Serial.parseFloat();
  // --- FIX: Clear the buffer after reading the float to remove the newline character ---
  while (Serial.available()) {
    Serial.read(); 
  }
  // -----------------------------------------------------------------------------------
  Serial.print("Light Weight (F1): ");
  Serial.print(F1_grams);
  Serial.println(" grams set.");
  
  // Wait for the user to input the heavy weight
  Serial.println("\nPlease enter the weight (in grams) of your heavy calibration object.");
  Serial.println("Example: 500.0");
  while (Serial.available() == 0) {
    delay(10);
  }
  F2_grams = Serial.parseFloat();
  // --- FIX: Clear the buffer after reading the float to remove the newline character ---
  while (Serial.available()) {
    Serial.read(); 
  }
  // -----------------------------------------------------------------------------------
  Serial.print("Heavy Weight (F2): ");
  Serial.print(F2_grams);
  Serial.println(" grams set.");
  
  // Proceed to the calibration measurements
  performCalibration();
  
  // --- After successful calibration, perform TARE ---
  // The TARE logic is moved to a separate function for reuse
  tareScale();
  
  // Initialize moving average array to zero
  for (int i = 0; i < AVG_WINDOW_SIZE; i++) {
    readings[i] = 0.0;
  }
}

// Function to calculate FSR resistance from analog reading
float calculateResistance_kOhm() {
  int analogReading = analogRead(FSR_PIN);
  float analogV_out = (analogReading / 1024.0) * V_IN; // Vout in Volts

  // Handle edge cases
  if (analogV_out < 0.01) { 
    // If Vout is very low, resistance is extremely high (no pressure)
    return PULL_R * 10.0 / 1000.0; // Return a very high kOhm value (100k)
  }
  if ((V_IN / analogV_out) <= 1.0) {
    // If Vout is near V_IN, resistance is near zero (max pressure)
    return 0.001; 
  }

  // R_FSR = R_pull * ((V_IN / V_out) - 1)
  float fsrResistance_Ohm = PULL_R * ((V_IN / analogV_out) - 1.0);
  return fsrResistance_Ohm / 1000.0; // Return in kOhms
}

// Helper function to calculate weight using calibrated constants
float calculateWeight_grams(float fsrResistance_kOhm) {
  // We allow calculation during TARE, so we don't block on !is_calibrated here.
  if (fsrResistance_kOhm == 0.0) {
    return 0.0;
  }
  
  // The threshold R_kOhm > 30.0 kOhm typically means no weight (or very light)
  if (fsrResistance_kOhm > 30.0) { 
    return 0.0;
  }
  
  // Apply the custom power law formula: F_g = (k / R_kOhm)^(1/gamma)
  float rawWeight = pow((FSR_POWER_K / fsrResistance_kOhm), FSR_GAMMA_EXPONENT);
  
  // Apply TARE offset (only when running, not during the TARE calculation itself)
  // This function is only used to CALCULATE the TARE offset or in the main loop.
  // The logic handles the subtraction in the main return line below.
  
  return rawWeight; // Return RAW weight for TARE calculation or main loop subtraction
}


void performCalibration() {
  // --- Measure R1 (Light Weight) ---
  Serial.println("\n--- Step 1: Measuring Light Weight (R1) ---");
  Serial.print("PLACE the ");
  Serial.print(F1_grams);
  Serial.println("g weight on the FSR. Press any key when ready...");
  
  while (Serial.available() == 0) {
    delay(10);
  }
  Serial.read(); // Clear buffer from the 'ready' input
  
  // Take several readings for stability
  float sumR1 = 0;
  for (int i = 0; i < 50; i++) {
    sumR1 += calculateResistance_kOhm();
    delay(10);
  }
  R1_kOhm = sumR1 / 50.0;
  Serial.print("Measured R1: ");
  Serial.print(R1_kOhm, 3);
  Serial.println(" kOhms");
  
  // --- Measure R2 (Heavy Weight) ---
  Serial.println("\n--- Step 2: Measuring Heavy Weight (R2) ---");
  Serial.print("REMOVE the light weight, then PLACE the ");
  Serial.print(F2_grams);
  Serial.println("g weight on the FSR. Press any key when ready...");
  
  while (Serial.available() == 0) {
    delay(10);
  }
  Serial.read(); // Clear buffer from the 'ready' input
  
  // Take several readings for stability
  float sumR2 = 0;
  for (int i = 0; i < 50; i++) {
    sumR2 += calculateResistance_kOhm();
    delay(10);
  }
  R2_kOhm = sumR2 / 50.0;
  Serial.print("Measured R2: ");
  Serial.print(R2_kOhm, 3);
  Serial.println(" kOhms");
  
  // --- Calculate Power Law Constants ---
  float logR_ratio = log(R2_kOhm) - log(R1_kOhm);
  float logF_ratio = log(F2_grams) - log(F1_grams);
  
  if (logF_ratio == 0) {
      Serial.println("\nERROR: Calibration weights were identical!");
      return;
  }
  
  float gamma = - logR_ratio / logF_ratio;
  
  // 2. Calculate k using R1 and F1: k = R1 / F1^-gamma
  FSR_POWER_K = R1_kOhm / pow(F1_grams, -gamma);
  
  // 3. The final exponent we use is 1/gamma
  FSR_GAMMA_EXPONENT = 1.0 / gamma;
  
  Serial.println("\n--- Calibration Complete ---");
  Serial.print("Calculated k (Numerator): ");
  Serial.println(FSR_POWER_K, 4);
  Serial.print("Calculated Exponent (1/gamma): ");
  Serial.println(FSR_GAMMA_EXPONENT, 4);
  
  is_calibrated = true;
  delay(500); 
}

// New function for Taring the scale
void tareScale() {
  if (!is_calibrated) return;
  
  Serial.println("\nTARE initiated: Removing all weights and settling for 2 seconds...");
  delay(2000); // Allow sensor to physically settle after interaction
  
  Serial.println("Taking 100-sample TARE average...");
  float sumTare = 0;
  
  // Use a longer average for a robust zero
  for (int i = 0; i < 100; i++) {
    // We call calculateWeight_grams which returns the RAW calculated weight
    sumTare += calculateWeight_grams(calculateResistance_kOhm()); 
    delay(10);
  }
  
  TARE_OFFSET_GRAMS = sumTare / 100.0;
  Serial.print("TARE Offset set to: ");
  Serial.print(TARE_OFFSET_GRAMS, 2);
  Serial.println(" grams. Zeroing complete.");
}


void loop() {
  if (!is_calibrated) {
    Serial.println("Waiting for calibration...");
    delay(1000);
    return;
  }
  
  // --- Check for Manual TARE Command ---
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'T' || command == 't') {
      tareScale();
    }
    // Clear any remaining characters (like newline)
    while (Serial.available()) Serial.read();
  }
  
  // --- RUN PHASE: Creep Mitigation ---

  // 1. Get the latest raw weight reading
  float raw_weight = calculateWeight_grams(calculateResistance_kOhm());
  
  // 2. Apply the TARE offset
  float instantaneous_weight = max(0.0, raw_weight - TARE_OFFSET_GRAMS);

  // 3. Store the reading in the moving average array
  readings[readingIndex] = instantaneous_weight;
  readingIndex = (readingIndex + 1) % AVG_WINDOW_SIZE;

  // 4. Calculate the average
  float sum = 0;
  for (int i = 0; i < AVG_WINDOW_SIZE; i++) {
    sum += readings[i];
  }
  float filtered_weight = sum / AVG_WINDOW_SIZE;
  
  // Display the calculated weight
  Serial.print("Filtered Weight: ");
  Serial.print(filtered_weight, 2); 
  Serial.println(" grams");

  delay(50); // Increased sampling rate for better averaging
}