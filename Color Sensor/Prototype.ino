#include <SPI.h> 
#include <SD.h>

// TCS230 or TCS3200 pins wiring to Arduino
#define S0 8
#define S1 9
#define S2 10
#define S3 11
#define sensorOut 12

// Calibration baseline 
int baselineClearPeriodCount = 0; // Establish this value during calibration

// Target count for 50ms
const int targetCount = 360; 

// Variables for counting periods within measurementTime
int redPeriodCount = 0;
int greenPeriodCount = 0;
int bluePeriodCount = 0;
int clearPeriodCount = 0;

// Timeout for pulseIn()
const int pulseTimeout = 2000; 

// Pin for SD card chip select - Built-in for Arduino Micro
const int chipSelect = 4;  

// Variables for SD card writing
File sensorDataFile;
String dataString = ""; 

void setup() {
  // Setting outputs for sensor control
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  // Setting sensorOut as input for receiving frequency data
  pinMode(sensorOut, INPUT);

  // Start serial communication for debugging output
  Serial.begin(9600);

  // Initialize SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    while(1); // Potential error handling loop
  }
  Serial.println("initialization done.");

  // Open file for writing
  sensorDataFile = SD.open("sensordata.txt", FILE_WRITE);
  if (!sensorDataFile) {
    Serial.println("Error opening file!");
    // Add error handling here
  }
}

void loop() {
  // --- Red Reading ---
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  startTime = millis();
  redPeriodCount = 0;

  // Dynamically adjust measurementTime
  unsigned long currentTime = millis(); 
  int dynamicMeasurementTime = 50; // Initial value
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

  // --- Green Reading --- (similarly modified)
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  startTime = millis();
  greenPeriodCount = 0; 
  // ... (same dynamic adjustment as for red)

  // --- Blue Reading --- (similarly modified)
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  startTime = millis();
  bluePeriodCount = 0; 
  // ... (same dynamic adjustment as for red)

  // --- Clear Reading (for Turbidity) ---
  digitalWrite(S2, HIGH);
  digitalWrite(S3, LOW);
  startTime = millis();
  clearPeriodCount = 0; 
  // ... (same dynamic adjustment as for red)

  // --- Turbidity Calculation ---
  int turbidityReductionPercent = (baselineClearPeriodCount - clearPeriodCount) * 100 / baselineClearPeriodCount;  

  // --- Build CSV data line to store ---
  dataString  = String(redPeriodCount) + "," + String(greenPeriodCount) + "," + 
                String(bluePeriodCount) + "," + String(clearPeriodCount) + "," + 
                String(turbidityReductionPercent) + "%\n";

  // --- Write to SD card ---
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
