#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

// Define pins connected to the TCS3200 sensor
#define S0 5
#define S1 6
#define S2 7
#define S3 8
#define sensorOut 9
#define shutdownPin 10  // Define the shutdown pin
#define greenLEDPin 11  // Define the green LED pin

// Calibration values for clear CSF readings (TO-DO: Calibrate these)
int baselineClearPeriodCount = 1; // Initialize to 1 to prevent division by zero
int baselineRedPeriodCount = 1;   // Initialize to 1 to prevent division by zero
int baselineGreenPeriodCount = 1; // Initialize to 1 to prevent division by zero
int baselineBluePeriodCount = 1;  // Initialize to 1 to prevent division by zero

// Target count to be achieved within the dynamic time adjustment
const int targetCount = 360; 

// Timeout setting for the pulseIn() function (microseconds)
const int pulseTimeout = 2000; 

// Specifies the chip select pin for the SD card module
const int chipSelect = 12;  

// Variables for interacting with the SD card
File sensorDataFile;
String dataString = ""; 

// Initialize the LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Change the address if needed

// Initialize the RTC
RTC_DS3231 rtc;

// Variable to track shutdown state
bool isShutdown = false;

// Variable to store patient ID
String patientID = "";

// Function Prototypes
void setupSensorPins();
void readColor(int &colorCount, int S2State, int S3State);
void checkShutdown();
void blinkLED(int delayTime);
String getTimestamp();
void displayDataOnLCD(const String &data);

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  // Give time for the serial monitor to open
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Serial communication initialized.");

  // Set up the sensor pins
  setupSensorPins();
  Serial.println("Sensor pins setup completed.");

  // Set up the shutdown pin and green LED pin
  pinMode(shutdownPin, INPUT_PULLUP); // Using internal pull-up resistor
  pinMode(greenLEDPin, OUTPUT);

  // Turn on the green LED to indicate standby mode
  digitalWrite(greenLEDPin, HIGH);

  // Initialize SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed or not present!");
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.print("SD card failed!");

    // Blink the green LED fast to indicate an error
    while (1) {
      blinkLED(200); // Fast blink
    }
  }
  Serial.println("Initialization done.");

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Initializing...");

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    lcd.clear();
    lcd.print("RTC failed!");
    while (1) {
      blinkLED(200); // Fast blink
    }
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Prompt for patient ID
  Serial.println("Please enter patient ID and press ENTER:");
  while (Serial.available() == 0) {
    // Wait for input
  }
  patientID = Serial.readStringUntil('\n');
  patientID.trim();
  Serial.print("Patient ID set to: ");
  Serial.println(patientID);
  lcd.clear();
  lcd.print("Patient ID: ");
  lcd.setCursor(0, 1);
  lcd.print(patientID);
}

void loop() {
  // Check if shutdown switch is pressed
  checkShutdown();

  // If the shutdown switch is not pressed, run the main loop
  if (!isShutdown) {
    // Indicate normal operation with long blinks
    digitalWrite(greenLEDPin, HIGH);
    delay(1000);
    digitalWrite(greenLEDPin, LOW);
    delay(1000);

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
    int turbidityPercent = (clearPeriodCount - baselineClearPeriodCount) * 100 / baselineClearPeriodCount;
    int redChangePercent = (redPeriodCount - baselineRedPeriodCount) * 100 / baselineRedPeriodCount;
    int greenChangePercent = (greenPeriodCount - baselineGreenPeriodCount) * 100 / baselineGreenPeriodCount;
    int blueChangePercent = (bluePeriodCount - baselineBluePeriodCount) * 100 / baselineBluePeriodCount;

    // Get the current timestamp
    String timestamp = getTimestamp();

    // Build the data string with timestamp and patient ID
    dataString  = patientID + "," + timestamp + "," + String(redPeriodCount) + "," + String(greenPeriodCount) + "," + 
                  String(bluePeriodCount) + "," + String(clearPeriodCount) + "," + 
                  String(turbidityPercent) + "," + 
                  String(redChangePercent) + "," + String(greenChangePercent) + "," +
                  String(blueChangePercent) + "%"; 

    // Debug statement to print the data string
    Serial.println("Data String: " + dataString);

    // Log data to SD card
    sensorDataFile = SD.open("SENSOR.TXT", FILE_WRITE);
    if (sensorDataFile) {
      sensorDataFile.println(dataString);
      sensorDataFile.close();
      Serial.println("Text file write complete");
    } else {
      // if the file didn't open, print an error:
      Serial.println("Error opening text file.");
    }

    // Display data on the LCD
    displayDataOnLCD(dataString);

    // Short delay before the next loop iteration
    delay(1000);
  } else {
    // If the shutdown switch is pressed, indicate standby mode
    digitalWrite(greenLEDPin, HIGH);
  }
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
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

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
  //Serial.print(", S3: ");
  //Serial.println(S3State);
  //Serial.print("): ");
  //Serial.println(colorCount);
}

void checkShutdown() {
  if (digitalRead(shutdownPin) == LOW) {
    // Shutdown switch pressed
    if (!isShutdown) {
      Serial.println("Shutdown switch pressed. Stopping SD card operations.");
      lcd.clear();
      lcd.print("Shutting down...");

      // Close the file if it's open
      if (sensorDataFile) {
        sensorDataFile.close();
      }

      // Turn off the green LED to indicate shutdown
      digitalWrite(greenLEDPin, LOW);

      isShutdown = true;
    }
  } else {
    // Shutdown switch released
    if (isShutdown) {
      Serial.println("Shutdown switch released. Restarting operations.");
      lcd.clear();
      lcd.print("Restarting...");

      // Turn on the green LED to indicate normal operation
      digitalWrite(greenLEDPin, HIGH);

      isShutdown = false;
    }
  }
}

void blinkLED(int delayTime) {
  digitalWrite(greenLEDPin, HIGH);
  delay(delayTime);
  digitalWrite(greenLEDPin, LOW);
  delay(delayTime);
}

String getTimestamp() {
  DateTime now = rtc.now();
  char buf[20];
  sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d", 
          now.year(), now.month(), now.day(), 
          now.hour(), now.minute(), now.second());
  return String(buf);
}

void displayDataOnLCD(const String &data) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(data.substring(0, 20));
  if (data.length() > 20) {
    lcd.setCursor(0, 1);
    lcd.print(data.substring(20, 40));
  }
  if (data.length() > 40) {
    lcd.setCursor(0, 2);
    lcd.print(data.substring(40, 60));
  }
  if (data.length() > 60) {
    lcd.setCursor(0, 3);
    lcd.print(data.substring(60, 80));
  }
}
