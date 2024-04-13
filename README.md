# BOSoMetre
 CSF Sensor Creation

Doctorate thesis for B. Bahadır Akbulut.

Project Title: Cerebrospinal Fluid Monitoring with TCS3200 Sensor and SD Card Logging

Project Overview:

This Arduino project aims to measure both the color and turbidity (clarity) of cerebro samples using a TCS3200 color sensor. The sensor data is logged onto an SD card for later analysis, making it suitable for potential early infection infection infection detection systems.

Hardware Components:

Arduino Micro (or another compatible Arduino board): The central controller.
TCS3200 Color Sensor: Detects light passing through the CSF and provides color and intensity data.
SD Card Module: Provides non-volatile storage for sensor readings.
Appropriate Resistors and LEDs: Used in the sensor and LED setup circuit (circuit diagram would be helpful for this)
CSF Sample Housing: A custom housing to integrate the sensor and LED to ensure consistent light path through the fluid.
Software Libraries

SPI.h: Required for communication with the SD card module on most Arduino boards.
SD.h: Provides functions for interacting with the SD card (initialization, file operations).
Code Breakdown

1. Header Includes and Pin Definitions

Includes necessary libraries for SD card operations (SPI.h and SD.h)
Defines Arduino pins connected to the TCS3200 sensor's S0, S1, S2, S3, and sensorOut lines.
Defines the chip select pin for the SD card module (chipSelect).
2. Global Variables

baselineClearPeriodCount: Stores the calibrated count value for clear CSF.
targetCount: Desired number of counts within the measurement timeframe.
redPeriodCount, greenPeriodCount, bluePeriodCount, clearPeriodCount: Store counts for each color filter and the clear reading.
pulseTimeout: Maximum wait time for the pulseIn() function.
sensorDataFile: File object representing the data file on the SD card.
dataString: Temporarily holds the formatted data to be written to the SD card.
3. setup() Function

Initializes serial communication (for debugging)
Initializes the SD card and opens a file named "sensordata.txt" for writing.
Performs any necessary sensor setup tasks.
4. loop() Function

Color Readings (Red, Green, Blue):

Sets the sensor configuration for the specific color filter.
Obtains period counts using pulseIn().
Dynamically adjusts measurementTime using an inner loop to achieve approximately the targetCount within 50ms.
Clear Reading (Turbidity):

Sets the sensor to read without filtering for turbidity calculation.
Measures period counts, again dynamically adjusting measurement time.
Turbidity Calculation:

Calculates percentage reduction in clear count compared to the calibrated baselineClearPeriodCount, indicating turbidity.
SD Card Writing:

Builds a comma-separated data string (dataString) with the period counts and turbidity.
Writes dataString to the SD card file.
5.  closeDataFile() Function

Closes the SD card data file gracefully (this function needs to be called at appropriate times in your overall project logic).
Important Considerations:

Calibration: Thorough calibration with known CSF samples is essential for accurate turbidity detection.
Error Handling: Implement error handling for SD card operations and sensor readings.
File Management: Strategically call closeDataFile() to ensure data integrity.