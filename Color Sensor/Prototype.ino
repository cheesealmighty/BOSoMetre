#include <SPI.h> 
#include <SD.h>

// Define pins connected to the TCS3200 sensor
#define S0 8
#define S1 9
#define S2 10
#define S3 11
#define sensorOut 12

// Store the calibration value for clear CSF readings
int baselineClearPeriodCount = 0; // Establish this value during calibration

// Specifies the target count to be achieved within the dynamic time adjustment
const int targetCount = 360; 

// Variables to store counts for each color and clear reading
int redPeriodCount = 0;
int greenPeriodCount = 0;
int bluePeriodCount = 0;
int clearPeriodCount = 0;

// Timeout setting for the pulseIn() function
const int pulseTimeout = 2000; // Timeout in microseconds

// Specifies the chip select pin for the SD card module (Arduino Micro) 
const int chipSelect = 4;  

// Variables for interacting with the SD card
File sensorDataFile;
String dataString = ""; 

void setup() {
  // Set up the pins for controlling the TCS3200 sensor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  // Set up the pin for receiving the sensor's output 
  pinMode(sensorOut, INPUT);

  // Begin serial communication for debugging purposes
  Serial.begin(9600);

  // Initialize SD card communication
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    while(1); // Potential error handling loop
  }
  Serial.println("initialization done.");

  // Open the data file on the SD card for writing
  sensorDataFile = SD.open("sensordata.txt", FILE_WRITE);
  if (!sensorDataFile) {
    Serial.println("Error opening file!");
    // Add error handling here
  }
}

void loop() {
  // --- Reading Red Color ---
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  unsigned long startTime = millis();
  redPeriodCount = 0;

  // Dynamic adjustment of measurement time
  unsigned long currentTime = millis(); 
  int dynamicMeasurementTime = 50; 
  while (currentTime - startTime < dynamicMeasurementTime) { 
      int pulseWidth = pulseIn(sensorOut, LOW, pulseTimeout);
      if (pulseWidth > 0) {
          redPeriodCount++; 
          if (redPeriodCount >= targetCount) {
              dynamicMeasurementTime = currentTime - startTime; 
              break; 
          }
      } 
      currentTime = millis();
  }

  // --- Reading Green Color ---
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  startTime = millis();
  greenPeriodCount = 0; 
  // Dynamic adjustment of measurement time 
  currentTime = millis(); 
  dynamicMeasurementTime = 50; 
  while (currentTime - startTime < dynamicMeasurementTime) { 
      int pulseWidth = pulseIn(sensorOut, LOW, pulseTimeout);
      if (pulseWidth > 0) {
          greenPeriodCount++; 
          if (greenPeriodCount >= targetCount) {
              dynamicMeasurementTime = currentTime - startTime; 
              break; 
          }
      } 
      currentTime = millis();
  }

  // --- Reading Blue Color ---
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  startTime = millis();
  bluePeriodCount = 0; 
  // Dynamic adjustment of measurement time
  currentTime = millis(); 
  dynamicMeasurementTime = 50; 
  while (currentTime - startTime < dynamicMeasurementTime) { 
      int pulseWidth = pulseIn(sensorOut, LOW, pulseTimeout);
      if (pulseWidth > 0) {
          bluePeriodCount++; 
          if (bluePeriodCount >= targetCount) {
              dynamicMeasurementTime = currentTime - startTime; 
              break; 
          }
      } 
      currentTime = millis();
  }

  // --- Reading Clear Channel (for Turbidity) ---
  digitalWrite(S2, HIGH);
  digitalWrite(S3, LOW);
  startTime = millis();
  clearPeriodCount = 0; 
  // Dynamic adjustment of measurement time
  currentTime = millis(); 
  dynamicMeasurementTime = 50; 
  while (currentTime - startTime < dynamicMeasurementTime) { 
      int pulseWidth = pulseIn(sensorOut, LOW, pulseTimeout);
      if (pulseWidth > 0) {
          clearPeriodCount++; 
          if (clearPeriodCount >= targetCount) {
              dynamicMeasurementTime = currentTime - startTime; 
              break; 
          }
      } 
      currentTime = millis();
  }

  // --- Calculate Turbidity (Reduction in Clear Reading) ---
  int turbidityReductionPercent = (baselineClearPeriodCount - clearPeriodCount) * 100 / baselineClearPeriodCount;  

  // --- Build the CSV data string for SD card logging ---
  dataString  = String(redPeriodCount) + "," + String(greenPeriodCount) + "," + 
                String(bluePeriodCount) + "," + String(clearPeriodCount) + "," + 
                String(turbidityReductionPercent) + "%\n";

  // --- Write the data to the SD card ---
  if (sensorDataFile) {
    sensorDataFile.print(dataString); 
  }
}

// (Important!) Close the file in some appropriate way
void closeDataFile() {
  if (sensorDataFile) {
    sensorDataFile.close();
    Serial.println("Data file closed.");
  } 
}
