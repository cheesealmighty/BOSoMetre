#include <SPI.h> 

// Define pins connected to the TCS3200 sensor
#define S0 2
#define S1 3
#define S2 4
#define S3 5
#define sensorOut 6



// Calibration values for clear CSF readings (TO-DO: Calibrate these)
int baselineClearPeriodCount = 1; // Initialize to 1 to prevent division by zero
int baselineRedPeriodCount = 1;   // Initialize to 1 to prevent division by zero
int baselineGreenPeriodCount = 1; // Initialize to 1 to prevent division by zero
int baselineBluePeriodCount = 1;  // Initialize to 1 to prevent division by zero

// Target count to be achieved within the dynamic time adjustment
const int targetCount = 360; 

// Timeout setting for the pulseIn() function (microseconds)
const int pulseTimeout = 2000; 

// Variables for interacting with the SD card
String dataString = ""; 

// Function Prototypes
void setupSensorPins();
void readColor(int &colorCount, int S2State, int S3State);

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  // Give time for the serial monitor to open
  delay(1000);
  Serial.println("Serial communication initialized.");

  // Set up the sensor pins
  setupSensorPins();
  Serial.println("Sensor pins setup completed.");
}

void loop() {
  // Debug statement to indicate start of loop
  Serial.println("Starting loop...");

  // Variables to store counts for each color and clear reading
  int redPeriodCount = 0;
  int greenPeriodCount = 0;
  int bluePeriodCount = 0;
  int clearPeriodCount = 0;

  // Read Red, Green, Blue, and Clear colors
  readColor(redPeriodCount, LOW, LOW);
  readColor(greenPeriodCount, HIGH, HIGH);
  readColor(bluePeriodCount, LOW, HIGH);
  readColor(clearPeriodCount, HIGH, LOW);

  // Calculate and log data
  int turbidityReductionPercent = (baselineClearPeriodCount - clearPeriodCount) * 100 / baselineClearPeriodCount;
  int redChangePercent = (redPeriodCount - baselineRedPeriodCount) * 100 / baselineRedPeriodCount;
  int greenChangePercent = (greenPeriodCount - baselineGreenPeriodCount) * 100 / baselineGreenPeriodCount;
  int blueChangePercent = (bluePeriodCount - baselineBluePeriodCount) * 100 / baselineBluePeriodCount;

  // Build and print the data string
  dataString  = String(redPeriodCount) + "," + String(greenPeriodCount) + "," + 
                String(bluePeriodCount) + "," + String(clearPeriodCount) + "," + 
                String(turbidityReductionPercent) + "," + 
                String(redChangePercent) + "," + String(greenChangePercent) + "," +
                String(blueChangePercent) + "%\n"; 

  // Debug statement to print the data string
  Serial.println("Data String: " + dataString);

  // Short delay before the next loop iteration
  delay(1000);
}

void setupSensorPins() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  
}

void readColor(int &colorCount, int S2State, int S3State) {
    // Setting frequency scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  
  digitalWrite(S2, S2State);
  digitalWrite(S3, S3State);
  unsigned long startTime = millis();
  colorCount = 0;
  int dynamicMeasurementTime = 50;
  unsigned long currentTime = millis(); 

  while (currentTime - startTime < dynamicMeasurementTime) {
    int pulseWidth = pulseIn(sensorOut, LOW, pulseTimeout); 
    if (pulseWidth > 0) {
      colorCount++; 
      if (colorCount >= targetCount) { 
        dynamicMeasurementTime = currentTime - startTime;
        break; 
      }
    } 
    currentTime = millis();
  }

  // Debug statement to print color count
  //Serial.print("Color count (S2: ");
  //Serial.print(S2State);
 // Serial.print(", S3: ");
  //Serial.println(S3State);
  //Serial.print("): ");
  //Serial.println(colorCount);
}
