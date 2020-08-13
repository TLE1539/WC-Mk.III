/*
Program that reads thermistor temps in Fahrenheit

Based on the Vishay NTCLG100E2 radial 220k thermistor

Each thermistor is configured as the lower resistor in a resistor voltage divider with an
upper resistor of value 'Rdiv', where the analog voltage inputs of the arduino are connected 
to the outputs of the voltage dividers

Created 7/15/2020
Authored by Tyler Le
*/
#include <math.h>

const int sampleCount = 5;  // number of samples to average
const int delayTime = 100;  // time delay between each sample taken
const boolean isCelsius = true; // choose whether to display temperature in celsius or fahrenheit

const int analogBitResolution = 1024;
const double Vsrc = 5.0;
const double Rdiv_t1 = 180400;  // thermistor1 is centered around 30C
const double Rdiv_t2 = 1796000; // thermistor2 is centered around -20C

double samples_t1[sampleCount];
double samples_t2[sampleCount];

void setup() {
  Serial.begin(115200);
}

void loop() {
  int Vardiv_t1 = analogRead(A0);
  double Vdiv_t1 = Vardiv_t1 * Vsrc / analogBitResolution;
  double Rtherm_t1 = (Vdiv_t1 * Rdiv_t1) / (Vsrc - Vdiv_t1);
  double avTemp_t1 = 0;

  int Vardiv_t2 = analogRead(A5);
  double Vdiv_t2 = Vardiv_t2 * Vsrc / analogBitResolution;
  double Rtherm_t2 = (Vdiv_t2 * Rdiv_t2) / (Vsrc - Vdiv_t2);
  double avTemp_t2 = 0;  
  
  // implement FIFO functionality into sample collection
  for (int i = sampleCount - 1; i > 0; i--){
    samples_t1[i] = samples_t1[i - 1];
    samples_t2[i] = samples_t2[i - 1];
  }
  
  if (isCelsius){
    samples_t1[0] = resistanceToTemperature(Rtherm_t1);
    samples_t2[0] = resistanceToTemperature(Rtherm_t2);
  }
  else{
    samples_t1[0] = celsiusToFahrenheit(resistanceToTemperature(Rtherm_t1));
    samples_t2[0] = celsiusToFahrenheit(resistanceToTemperature(Rtherm_t2));
  }
  
  for(double sample : samples_t1){
    avTemp_t1 += sample;
  }
  
  for(double sample : samples_t2){
    avTemp_t2 += sample;
  }
  
  avTemp_t1 /= sampleCount;
  avTemp_t2 /= sampleCount;
  
  Serial.print("hot: ");
  Serial.println(avTemp_t1);
  // Serial.println(Vardiv_t1);
  Serial.print("cold: ");
  Serial.println(avTemp_t2);
  // Serial.println(Vardiv_t2);

  delay(delayTime);
}

/* 
Converts a resistance in ohms to a corresponding temperature in celsius using
a piecewise function that models the thermistor's temp vs. resistance lookup table
in the datasheet

Equations are calculated from dcode.fr/equation-finder

Accurate within +- 0.1C from -40C to 40C
*/
static double resistanceToTemperature(double resistance){
  double t = 0.0;
  double r = resistance / 1000000; // For simplified convertion process
  if (r <= 0.33320808){
    // covers 40C to 15C
    // table values: -25 to 30
    t = (-496.824 * pow(r, 11)) + (5047.59 * pow(r, 10)) - (22183.3 * pow(r, 9))
      + (55779.1 * pow(r, 8)) - (89449.5 * pow(r, 7)) + (96536.8 * pow(r, 6))
      - (72101.2 * pow(r, 5)) + (37738.9 * pow(r, 4)) - (13875.9 * pow(r, 3))
      + (3580.8 * pow(r, 2)) - (672.143 * r) + 87.0692;
  }
  
  else if (r <= 1.0445345 && r > 0.33320808){
    // covers 15C to -10C
    t = (-0.125514 * pow(r, 11)) + (2.83242 * pow(r, 10)) - (27.343 * pow(r, 9))
      + (149.323 * pow(r, 8)) - (514.217 * pow(r, 7)) + (1178.43 * pow(r, 6))
      - (1848.56 * pow(r, 5)) + (2010.73 * pow(r, 4)) - (1520.96 * pow(r, 3))
      + (800.012 * pow(r, 2)) - (303.851 * r) + 65.2518;
  }
  
  else{
    // covers -10C to -40C
    // table values: -40 to -10
    t = (0.0176519 * pow(r,6)) - (.354364 * pow(r, 5)) + (2.92849 * pow(r, 4)) - (13.033 * pow(r, 3))
      + (34.5617 * pow(r, 2)) - (59.8321 * r) + 27.2221;
  }
  
  return t;
}

static double celsiusToFahrenheit(double c){
  return (c * 9 / 5) + 32;
}
