#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP183_U.h>     // Pressure Sensor
#include <Adafruit_TSL2561_U.h>    // Light Sensor
#include <Wire.h>
#include <DHT.h>                   // Humidity / Temperature library.

/**************************************************************************/
/*
                          4 Sensor + 1 Controller

The following arduino program will read the output of 4 sensors and 
communicate the information over a serial port using json to a server.  
This program will also listen to commands from the server and control a 
single digital port.

This software is provided as an example and can be modified to suit 
different sensors and/or expanded to control more digital ports.
    
*/
/**************************************************************************/

// Unique identifier used to know which arduino is broadcasting 
// and used to identify a port for sending commands.
String ARDUINO_NODE_NUMBER = "A199827";

// Time between cycles.  This millisecond number is added
// to the looping.  Inner loop for serial communication detection.
// Outer loop for sending general information.
int    CYCLES = 1000;
int    TIME_BETWEEN_CYCLES = 10;

/**************************************************************************/
/*
    Pressure Sensor
*/
/**************************************************************************/

// Uses digital pins 10, 11 and 12.
#define BMP183_CS   10

// Define Library
Adafruit_BMP183_Unified bmp = Adafruit_BMP183_Unified(BMP183_CS);  // use hardware SPI

/**************************************************************************/
/*
    Light Sensor
*/
/**************************************************************************/

// Uses analog pins 4 and 5.  

// Define Library
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

/**************************************************************************/
/*
    Temperature Humidity Sensor
*/
/**************************************************************************/

// Uses digital pin 2.
#define DHTPIN 2     // what digital pin we're connected to

// Defines the type of sensor that we are using.
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

/**************************************************************************/
/*
    Temperature Thermister
*/
/**************************************************************************/

// Uses analog pin A0
#define THERMISTORPIN A0         
// resistance at 25 degrees C - verify for more accuracy.
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor - verify for more accuracy.
#define SERIESRESISTOR 10000 

/**************************************************************************/
/*
    Output
*/
/**************************************************************************/
boolean switch1     = false;
String  switch1name = "switch1";
char    switch1on   = 121; // y
char    switch1off  = 122; // z
int     switch1pin  = 4;   // Arduino Board Digital Pin Number
int     switch1Cycles      = 0;
char    switch1CycleChar   = 0;


/**************************************************************************/
/*
    Debug
*/
/**************************************************************************/
boolean DEBUG = false;

/**************************************************************************/
/*
    Global
*/
/**************************************************************************/
int    samples[NUMSAMPLES];

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Pressure Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

    /* Initialise the sensor */
  if(!tsl.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  if (DEBUG) {
    displayLightSensorDetails();
  }
  
  dht.begin();

  /* Setup the sensor gain and integration time */
  configureLightSensor();

  /* Setup output pin */
  pinMode(switch1pin, OUTPUT);

}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/
void loop(void) 
{

  // Inner loop giving swich faster response time.
  for (int i = 0; i < CYCLES; i++) {

    // Read the character buffer;
    String recv;
    if (Serial.available() > 0) {
      //recv = Serial.read();
      recv = Serial.readString();
    }

    // Provide time delay functionality.
    if (recv.length() > 1) {
        String time = recv.substring(1, recv.length()); // Get the trailing numbers.
        switch1Cycles = time.toInt() * 100; // 100 multiplier gives us about 1 second units.
        switch1CycleChar = recv.charAt(0);  // Keep the cycle character in memory for switch purposes.
    }

    // Turn on switch variable
    // - when the correct character is received and the switch is off, or
    // - when the wrong character is received and the cycle counter is 1.
    if ((recv.charAt(0) == switch1on && switch1 == false) 
      || (switch1Cycles == 1 && switch1CycleChar == switch1off)) {
      switch1 = true;
      displayHeader(ARDUINO_NODE_NUMBER);
      displaySwitchDetails(switch1, switch1name, switch1on, switch1off);
      displayFooter();
    } 
    // Turn off switch variable
    // - when the correct character is received and the switch is off, or
    // - when the wrong character is received and the cycle counter is 1.
    if ((recv.charAt(0) == switch1off && switch1 == true) 
      || (switch1Cycles == 1 && switch1CycleChar == switch1on)) {
      switch1 = false;
      displayHeader(ARDUINO_NODE_NUMBER);
      displaySwitchDetails(switch1, switch1name, switch1on, switch1off);
      displayFooter();
    }

    // Actual board pin setting.
    if (switch1) {
      digitalWrite(switch1pin, HIGH);
    } else {
      digitalWrite(switch1pin, LOW);
    }

    // Reduce the cycle by one turn.
    if (switch1Cycles > 0) {
      switch1Cycles--;
    }

    // TODO - try different cycle times to accelerate response time.
    delay(TIME_BETWEEN_CYCLES);
  }

  // Outer loop giving sensor values.
  
  /* Get light sensor event */ 
  double lux;
  sensors_event_t lightEvent;
  tsl.getEvent(&lightEvent);
  if (lightEvent.light)
  {
    lux = lightEvent.light;
  }

  // Return nothing if the sensor is overloaded or
  // disconnected.
  if (lux == 0 || lux == 65536) {
    lux = NAN;
  }

  /* Get presure sensor event */
  float pressure;
  float pressureSensorTemp;
  sensors_event_t presureEvent;
  bmp.getEvent(&presureEvent);
  if (presureEvent.pressure)
  {
    pressure = presureEvent.pressure;
    pressureSensorTemp = bmp.getTemperature();
  }

  /* Temperature Humidity Sensor */
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  // Calculate the Thermister Temperature in Celcius.
  float steinhart = thermisterCalc();

  // Print the output to the serial port.
  displayHeader(ARDUINO_NODE_NUMBER);
  if (!isnan(h)) {
    Serial.print("{\"sensor\":\"humidity\", ");
    Serial.print("\"value\":");
    Serial.print(h);
    Serial.print(", ");
    Serial.print("\"unit\":\"percent\"},");
  }
  if (!isnan(t)) {
    Serial.print("{\"sensor\":\"temperature\", ");
    Serial.print("\"value\":");
    Serial.print(t);
    Serial.print(", ");
    Serial.print("\"unit\":\"celcius\"},");
  }
  if (!isnan(hic)) {
    Serial.print("{\"sensor\":\"heat index\", ");
    Serial.print("\"value\":");
    Serial.print(hic);
    Serial.print(", ");
    Serial.print("\"unit\":\"celcius\"},");
  }
  if (!isnan(steinhart)) {
    Serial.print("{\"sensor\":\"temperature probe\", ");
    Serial.print("\"value\":");
    Serial.print(steinhart);
    Serial.print(", ");
    Serial.print("\"unit\":\"celcius\"},");
  }
  if (!isnan(pressure)) {
    Serial.print("{\"sensor\":\"pressure\", ");
    Serial.print("\"value\":");
    Serial.print(pressure);
    Serial.print(", ");
    Serial.print("\"unit\":\"hPa\"},");
  }
  if (!isnan(lux)) {
    Serial.print("{\"sensor\":\"light\", ");
    Serial.print("\"value\":");
    Serial.print(lux);
    Serial.print(", ");
    Serial.print("\"unit\":\"lux\"},");
  }
  if (!isnan(pressureSensorTemp)) {
    Serial.print("{\"sensor\":\"temperature from pressure sensor\", ");
    Serial.print("\"value\":");
    Serial.print(pressureSensorTemp);
    Serial.print(", ");
    Serial.print("\"unit\":\"celcius\"},");
  }

  // Switch 1
  displaySwitchDetails(switch1, switch1name, switch1on, switch1off);

  // Footer
  displayFooter();

  // Wait until we cycle again!
  // delay(TIME_BETWEEN_CYCLES);
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2561
*/
/**************************************************************************/
void configureLightSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
}

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displayLightSensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
  Serial.println("------------------------------------");
  Serial.println("");
}

/**************************************************************************/
/*
    Calculate the Thermister temperature in celcuis.
*/
/**************************************************************************/
float thermisterCalc() {

  analogReference(EXTERNAL);

  // Thermister Calculation.
  uint8_t i;
  float average;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
 
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

  return steinhart;
  
}

/**************************************************************************/
/*
    Displays switch information.  Done as main loop and on state change.
*/
/**************************************************************************/
void displaySwitchDetails(boolean switchValue, String switchName, char charOn, char charOff)
{
  // Switch 1
  Serial.print("{\"switch\":\"");
  Serial.print(switchName);
  Serial.print("\", ");
  Serial.print("\"value\":");
  Serial.print(switchValue);
  Serial.print(", ");
  Serial.print("\"charOn\":\"y\", \"charOff\":\"z\"}");
}

/**************************************************************************/
/*
    Displays header information.  Important to indentify arduino.
*/
/**************************************************************************/
void displayHeader(String arduinoNodeNumber)
{
  Serial.print("{");
  Serial.print("\"nodeUID\":\"");
  Serial.print(arduinoNodeNumber);
  Serial.print("\", \"measurements\":[");
}
/**************************************************************************/
/*
    Displays footer information.  
*/
/**************************************************************************/
void displayFooter(void)
{
  Serial.println("]}");
}
