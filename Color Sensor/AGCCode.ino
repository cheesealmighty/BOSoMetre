#include <SPI.h>
#include <SD.h>

// Define pins connected to the TCS3200 sensor
#define S0 8
#define S1 9
#define S2 10
#define S3 11
#define sensorOut 12

// Pin for the SD card's chip select
const int chipSelect = 4; 

// Constants
const int measurementTime = 50; 
const int targetCount = 360;  
const int potPin = A0; // Potentiometer input pin

// Variables
int clearPeriodCount = 0;
int potValue = 0; 
float errorSum = 0; 
const float filterFactor = 0.1; 
File sensorDataFile;

void setup() {
  // Set up the TCS3200 sensor pins
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, LOW);
  pinMode(sensorOut, INPUT);

  // Initialize SD Card
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;  // Stop if SD card fails 
  }
  Serial.println("initialization done.");

  // Open the data file 
  sensorDataFile = SD.open("sensordata.csv", FILE_WRITE);
  if (!sensorDataFile) {
    Serial.println("Error opening file!");
    return;  // Stop if file opening fails 
  }

  // Write CSV header (optional)
  sensorDataFile.println("Time (ms),Clear Period Count,Potentiometer Value,Error");
}

void loop() {
  unsigned long startTime = millis();
  unsigned long currentTime;

  // Data acquisition for 1 minute (60000 milliseconds)
  while (currentTime - startTime < 60000) {   
    // Potentiometer Reading
    potValue = analogRead(potPin);  

    // Clear Reading 
    digitalWrite(S2, HIGH);
    digitalWrite(S3, LOW);
    clearPeriodCount = 0;  

    // Count periods within the fixed measurement time
    while (millis() - startTime < measurementTime) { 
        int pulseWidth = pulseIn(sensorOut, LOW, 2000); 
        if (pulseWidth > 0) {
            clearPeriodCount++; 
        } 
    }

    // AGC Brightness Adjustment
    int error = targetCount - clearPeriodCount; 
    errorSum = errorSum * (1 - filterFactor) + error * filterFactor; 

    // Adjust brightness based on error and potentiometer value
    float brightnessFactor = potValue / 1023.0; 
    int adjustedError = error * brightnessFactor; 

    // Save data to CSV file
    currentTime = millis();
    sensorDataFile.print(currentTime);
    sensorDataFile.print(",");
    sensorDataFile.print(clearPeriodCount);
    sensorDataFile.print(",");
    sensorDataFile.print(potValue);
    sensorDataFile.print(",");
    sensorDataFile.println(adjustedError);

  } 

  // Close the data file
  sensorDataFile.close();
}
