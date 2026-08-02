#include <Arduino.h>
#include <Wire.h>
#include "PCF8591.h"
#include <math.h> 

PCF8591 pcf(0x48);

const float VCC = 3.3;          
const int ADC_MAX = 255;        

// Thermistor Calibration Constants
const float SERIES_RESISTOR = 10000.0; 
const float THERMISTOR_NOMINAL = 10000.0;
const float TEMPERATURE_NOMINAL = 25.0;
const float B_COEFFICIENT = 7200.0;    

// Non-blocking Timer Variables
unsigned long lastPrintTime = 0;        
const unsigned long printInterval = 500; 

// --- Moving Average Filter Configuration ---
const int WINDOW_SIZE = 5;

// Buffers to store the last 5 readings
uint8_t potBuffer[WINDOW_SIZE]  = {0};
uint8_t lightBuffer[WINDOW_SIZE] = {0};
uint8_t extBuffer[WINDOW_SIZE]   = {0};
uint8_t tempBuffer[WINDOW_SIZE]  = {0};

int bufferIndex = 0; // Tracks the current position in the rolling arrays

void setup() {
  Serial.begin(115200);
  while (!Serial); 
  
  Serial.println("Initializing I2C...");
  Wire.begin(21, 22);

  if (pcf.begin()) {
    Serial.println("PCF8591 detected successfully!");
  } else {
    Serial.println("PCF8591 not found. Check wiring.");
    while (1); 
  }
}

void loop() {
  // 1. Continuous Hardware Sampling into Moving Average Buffers
  potBuffer[bufferIndex]   = pcf.read(0); // AIN0: Potentiometer
  lightBuffer[bufferIndex] = pcf.read(1); // AIN1: Light Sensor
  extBuffer[bufferIndex]   = pcf.read(2); // AIN2: Floating Pin
  tempBuffer[bufferIndex]  = pcf.read(3); // AIN3: Thermistor

  // Advance the index and roll over if we hit the window limit (0 to 4)
  bufferIndex++;
  if (bufferIndex >= WINDOW_SIZE) {
    bufferIndex = 0;
  }

  // 2. Non-Blocking Output Processing (Twice per second)
  unsigned long currentMillis = millis();
  if (currentMillis - lastPrintTime >= printInterval) {
    lastPrintTime = currentMillis; 

    // Sum up the last 5 readings for each channel
    float potSum = 0;
    float lightSum = 0;
    float extSum = 0;
    float tempSum = 0;

    for (int i = 0; i < WINDOW_SIZE; i++) {
      potSum   += potBuffer[i];
      lightSum += lightBuffer[i];
      extSum   += extBuffer[i];
      tempSum  += tempBuffer[i];
    }

    // Calculate the averages (as floating points for smoother scaling)
    float avgRawPot   = potSum / WINDOW_SIZE;
    float avgRawLight = lightSum / WINDOW_SIZE;
    float avgRawExt   = extSum / WINDOW_SIZE;
    float avgRawTemp  = tempSum / WINDOW_SIZE;

    // 3. Scale the Averaged Values
    float potVoltage = (avgRawPot / (float)ADC_MAX) * VCC;
    float extVoltage = (avgRawExt / (float)ADC_MAX) * VCC;
    float lightPercent = ((ADC_MAX - avgRawLight) / (float)ADC_MAX) * 100.0;

    // Prevent edge anomalies on the temperature channel
    if (avgRawTemp <= 0) avgRawTemp = 1;
    if (avgRawTemp >= ADC_MAX) avgRawTemp = ADC_MAX - 1;

    // Apply the VCC-tied thermistor calculations to the filtered value
    float thermistorResistance = SERIES_RESISTOR / (((float)ADC_MAX / avgRawTemp) - 1.0);
    float steinhart;
    steinhart = thermistorResistance / THERMISTOR_NOMINAL;     
    steinhart = log(steinhart);                                
    steinhart /= B_COEFFICIENT;                                
    steinhart += 1.0 / (TEMPERATURE_NOMINAL + 273.15);        
    steinhart = 1.0 / steinhart;                               
    float tempCelsius = steinhart - 273.15;                    

    // 4. Print Results to Serial Monitor
    Serial.print("Pot(raw): ");     
    Serial.print(potVoltage, 2);   
    Serial.print(" V (");           
    Serial.print(avgRawPot, 1);     // Prints the filtered average raw value
    Serial.print(")");

    Serial.print(" | Light(raw): ");     
    Serial.print(lightPercent, 1); 
    Serial.print(" % (");
    Serial.print(avgRawLight, 1);
    Serial.print(")");

    Serial.print(" | Temp(raw): ");      
    Serial.print(tempCelsius, 1);  
    Serial.print(" °C (");
    Serial.print(avgRawTemp, 1);
    Serial.print(")");

    Serial.print(" | External(raw): ");  
    Serial.print(extVoltage, 2);   
    Serial.print(" V (");
    Serial.print(avgRawExt, 1);
    Serial.println(")");
  }
}
