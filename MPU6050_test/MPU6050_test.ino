#include "FastIMU.h"
#include <Wire.h>

#define IMU_ADDRESS 0x69
#define PERFORM_CALIBRATION
MPU6050 IMU;
calData calib = { 0 };
AccelData accelData;
GyroData gyroData;
MagData magData;
void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Wire.setClock(400000);
  Serial.begin(115200);
  while (!Serial){ //we wait for Serial to give us a signal indiciating we have data streaming through the i2c
    ; 
  }
  int err = IMU.init(calib, IMU_ADDRESS);
  if (err !=0 ){
    Serial.print("Error initializing IMU:");
    Serial.println(err);
    //while (true){ //error initializing IMU, so we get stuck in this while loop
    //  ;
    //}
  }
#ifdef PERFORM_CALIBRATION
  Serial.println("FastIMU calibration & data example");
  if (IMU.hasMagnetometer()){
    delay(1000);
    Serial.println("Move IMU in figure 8 pattern until done.");
    delay(3000);
    IMU.calibrateMag(&calib);
    Serial.println("Magnetic Calibration done!");
  }
  else{
    delay(2000);
  }
  delay(5000);
  Serial.println("Keep IMU level.");
  delay(5000);
  IMU.calibrateAccelGyro(&calib);
  Serial.println("Calibration done!");
  Serial.println("Accel biases X/Y/Z: ");
  Serial.print(calib.accelBias[0]);
  Serial.print(", ");
  Serial.print(calib.accelBias[1]);
  Serial.print(", ");
  Serial.println(calib.accelBias[2]);
  Serial.println("Gyro biases X/Y/Z: ");
  Serial.print(calib.gyroBias[0]);
  Serial.print(", ");
  Serial.print(calib.gyroBias[1]);
  Serial.print(", ");
  Serial.print(calib.gyroBias[2]);
  if (IMU.hasMagnetometer()) {
    Serial.println("Mag biases X/Y/Z: ");
    Serial.print(calib.magBias[0]);
    Serial.print(", ");
    Serial.print(calib.magBias[1]);
    Serial.print(", ");
    Serial.println(calib.magBias[2]);
    Serial.println("Mag Scale X/Y/Z: ");
    Serial.print(calib.magScale[0]);
    Serial.print(", ");
    Serial.print(calib.magScale[1]);
    Serial.print(", ");
    Serial.println(calib.magScale[2]);
  }
  delay(5000);
  IMU.init(calib, IMU_ADDRESS);
#endif
  err = IMU.setGyroRange(500);
  err = IMU.setAccelRange(2);
if (err != 0) {
  Serial.print("Error setting range: ");
  Serial.println(err);
  while (true){
    ;
  }
}
}
void loop() {
  // put your main code here, to run repeatedly:
  IMU.update();
  IMU.getAccel(&accelData); //acceldata clss has 3 axes accelX, accelY and accelZ
  Serial.print(accelData.accelX);
  Serial.print("\t");
  Serial.print(accelData.accelY);
  Serial.print("\t");
  Serial.print(accelData.accelZ);
  Serial.print("\t");
  IMU.getGyro(&gyroData);
  Serial.print(gyroData.gyroX);
  Serial.print("\t");
  Serial.print(gyroData.gyroY);
  Serial.print("\t");
  Serial.print(gyroData.gyroZ);
  if (IMU.hasTemperature()) {
	  Serial.print("\t");
	  Serial.println(IMU.getTemp());
  }
  else {
    Serial.println();
  }
  delay(50);
}
