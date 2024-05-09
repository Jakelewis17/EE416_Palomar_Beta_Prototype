/*******************************************************************************
 * Programmers: Jake Lewis, Zachary Harrington, Nicholas Gerth, Matthew Stavig *                                                      
 * Class: EE416 - Electrical Engineering Design                                *
 * Sponsoring Company: Philips                                                 *
 * Industry Mentor: Scott Schweizer                                            *
 * Faculty Mentor: Mohammad Torabi                                             *
 *                                                                             *
 *                          Patient Monitor Project                            *
 *                                                                             *
 * Date: 5/8/2024                                                              *
 * File: blood_pressure.cpp                                                    *
 *                                                                             *
 * Description: Contains code to run the blood pressure system, interpret the  *
 *              data, and send data back to the app                            *                       
 *                                                                             *
 *                                                                             *
 ******************************************************************************/


#include "patient_monitor.h"

/* Extern variables */
extern int what_press;
extern patientdata Patientdata;
extern BlynkWifi Blynk;
extern BlynkTimer timer;

/* Global Variables */
float pressureVolt = 100;
int starttime_bp = 0, endtime_bp = 0;
extern int bp_control_value;
char temp_bp_number[] = "120/80 ";
int finalSystolic = 0;
int finalDiastolic = 0;

//Blynk Timer function
void BP_timer() 
{
  Blynk.virtualWrite(V55, Patientdata.BP); 
}


/***********************************************************************************************************
    Function: read_bp()
    Description: Sets up Blynk timer and branches to bp measurement function
    Preconditions: None
    Postconditions: BP Blynk timer set up
************************************************************************************************************/
void read_bp()
{
  //setup timer and enter main loop
  timer.setInterval(1000L, BP_timer); 
  app_main();

  //Reset button and write message to app
  bp_control_value = 0;
  Blynk.virtualWrite(V52, bp_control_value);
  Blynk.virtualWrite(V58, "Measurement Complete, View Server for Details or Measure Again");  //ecg
  
}


#include <vector>
#include <iostream>

extern "C"
{
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
}

const int motor_A1 = 12;
const int motor_B1 = 13;
const int valveSwitch = 25;
const int pVIn = 35;

// Data for processing voltage from pressure sensor
double rawSensorVal = 0.0;
double tempSensorVal = 0.0;
double divSensorVal = 0.0;
double pressureVal = 0.0;

// global sensor timing for multiple runs
double sigTime = 0.0;

// Final systolic and diastolic values
double systolicDbl = 0.0;
double diastolicDbl = 0.0;


// Structure to hold voltage and timing
struct bloodData {
  double voltage = 0.0;
  double time = 0.0;
};

// Create vectors from struct to process data
bloodData data;
std::vector<bloodData> bloodOutput;
std::vector<bloodData> systolicPoints;
std::vector<bloodData> diastolicPoints;
std::vector<bloodData> avgVec;
std::vector<bloodData> avgEVec;
std::vector<bloodData> oscNoise;


extern "C"
void app_main(void)
{
	init(); // Initialize ESP, pins, PWM 
	
	openValve();
	
	delay(10000);
	
	cycleBPSystem();
}

/***********************************************************************************************************
    Function: init()
    Description: Initializes the pins and valve status
    Preconditions: None
    Postconditions: System is ready to get pressure
************************************************************************************************************/
void init() {
 

  // Initiate A1 and A2 for air pump
  pinMode(motor_A1, OUTPUT);
  pinMode(motor_B1, OUTPUT);

  // Initiate pin valveSwitch for air valve as PWM with freq 10k, 12 bits
  ledcAttach(valveSwitch, 10000, 12);

  // Set air pump to stop
  digitalWrite(motor_A1, LOW);
  digitalWrite(motor_B1, LOW);

  // initialize valve as closed
  digitalWrite(valveSwitch, HIGH);

}


/***********************************************************************************************************
    Function: cycleBPSystem()
    Description: THe main flow of operations: Sets up pump, inflates cuff, samples the pressure, opens the 
                 valve, finds the blood pressure points, and converts them
    Preconditions: None
    Postconditions: BP points are found
************************************************************************************************************/
void cycleBPSystem()
{
  // How long data is collected for after pump inflates
  double seconds = 40.0;
	
  // clears vectors
  bloodOutput.clear();
  systolicPoints.clear();
  diastolicPoints.clear();
  
  // clears control vectors
  avgVec.clear();
  avgEVec.clear();
  oscNoise.clear();
  
  // reset time and final values
  sigTime = 0.0;
  finalSystolic = 0.0;
  finalDiastolic = 0.0;
  
  //Serial.println("Vectors cleared.");
  
  // closes valve
  closeValve();
  
  // set and unset pump to ensure full power
  delay(1000);
  pumpOn();
  delay(100);
  pumpOff();
  delay(100);
  pumpOn();
  
  //Serial.println("Pump cycled.");
  delay(2000);

  // inflate to ensure blood gets cut off
  while(getPressure() < 2665.00) {}

  //Serial.println("Exited loop");
  pumpOff();

  // sample pressure from the signal for seconds
  samplePressureSig(seconds);

  // release pressure
  openValve();

  // find the systolic and diastolic points from the main vector, with a windowSize of 10 for seconds
  findBPPoints(bloodOutput, 10, seconds);

  // set tSys and tDia to find the max and min values in the systolicPoints and diastolicPoints vectors
  double tSys, tDia = 2000.0;
  for(int i = 0; i < systolicPoints.size(); i++)
  {
	  tSys = max(tSys, systolicPoints[i].voltage);
  }
  for(int i = 0; i < diastolicPoints.size(); i++)
  {
  	  tDia = min(tDia, diastolicPoints[i].voltage);
  }
  
  // turn the mV to mmHg
  systolicDbl = convertMVtommHG(tSys);
  diastolicDbl = convertMVtommHG(tDia);
  
  // converts from double to int, adding 0.5 to round and truncate
  finalSystolic = systolicDbl + 0.5;
  finalDiastolic = diastolicDbl + 0.5;
  
  Serial.print("\n\nFinal Systolic in mmHg: ");
  Serial.println(finalSystolic);
  Serial.print("Final Diastolic in mmHg: ");
  Serial.println(finalDiastolic);
  Serial.println("\n");
  
  // release pressure
  openValve();
  delay(10000);
  closeValve();

  sprintf(Patientdata.BP, "%d", finalSystolic, "/", "%d", finalDiastolic);
  timer.run();

}

/***********************************************************************************************************
    Function: findBPPoints()
    Description: Finds the systolic and diastolic blood pressure from an input vector
    Parameters: The blood pressure data as a vector, the the window size for accuracy, and how long the 
                reading took in seconds 
    Preconditions: Data must have been sampled already to form a vector
    Postconditions: The blood pressure is returned 
************************************************************************************************************/
void findBPPoints(std::vector<bloodData> v, int windowSize, double s)
{
  // control variables
  bloodData t;
  int vecSize = v.size();

  // loops through the vector, calculating averages and other vectors
  for (int i = 0; i < vecSize; i++)
  {  
	// sets summations for averages
    double sum = 0.0, sumE = 0.0;
    
    // calculates average with length of window size, smoothing the signal
    for(int k = i - windowSize; k <= i + windowSize; ++k)
    {
    	sum += v[k].voltage;
    }
    
    // calculates the average with twice the window length, ensuring the signal is much more smooth and averaged out
    for(int k = i - (2 * windowSize); k <= i + (2 * windowSize); ++k)
    {
        sumE += v[k].voltage;
    }
        
    // calculates the averages
    double avg = sum / (2 * windowSize + 1);
    double avgE = sumE / (4 * windowSize + 1);
   
    // ensures that any 0s does not effect the final vectors, as would change the averaging if theres some points with no value
    if(v[i].time >= 0.5 && v[i].time <= s - 0.51)
    {
    	// sets values
		t.voltage = avg;
		t.time = v[i].time;
		avgVec.push_back(t);
		
		t.voltage = avgE;
		t.time = v[i].time;
		avgEVec.push_back(t);
		
		// this is to find the oscillation noise, which is used to find where the systolic and diastolic points are
		t.voltage = (avg - avgE);
		t.time = v[i].time;
		oscNoise.push_back(t);
    }
  }
  
  for(int i = 0; i < avgVec.size(); i++)
  {
	  // systolic points are when the noise is larger than 10
	  if(oscNoise[i].voltage >= 10)
	  {
		  t.voltage=avgVec[i].voltage;
		  t.time=avgVec[i].time;
		  systolicPoints.push_back(t);
	  }
	  // diasotlic points are on the drop, less than -10
	  else if(oscNoise[i].voltage <= -10)
	  {
		  t.voltage=avgVec[i].voltage;
		  t.time=avgVec[i].time;
		  diastolicPoints.push_back(t);
	  }
  }

}

/***********************************************************************************************************
    Function: convertMVtommHG()
    Description: Converts the millivolt reading of the pressure into a mm of mercury reading
    Parameters: The millivolt reading of the pressure
    Preconditions: None
    Postconditions: The reading is converted
************************************************************************************************************/
double convertMVtommHG(double mv)
{
  double out = 0.0;
  
  out = mv / 23.4545; //2.2k resistor gives a gain of 23.4545 (1 + 49.4k/2.2k)
  out = out - 35; // get rid of the voltage offset in the pressure transducer
  out = out / 3.5; // converts from mV to kPa
  out = out * 7.5; // converts from kPa to mmHg
  
  return out;
}

/***********************************************************************************************************
    Function: samplePressureSig()
    Description: Samples the outgoing signal for a period of time
    Parameters: The number of seconds the pressure will be sampled for 
    Preconditions: Pump must be inflated enough to have a readable pressure
    Postconditions: The pressure of the cuff in mV
************************************************************************************************************/
void samplePressureSig(int s)
{
  // sets the pwm at 3800/4096 duty, gives time for signal to correct
  ledcWrite(valveSwitch, 3800);
  delay(2000);

  // control variable for sampling once every 25 ms (25 * 40 = 1000 -> 1 second)
  int x = s * 40;
  
  // sets duty on valve to 3642/4096, giving constant and good outflow
  ledcWrite(valveSwitch, 3642);

  // samples at rate of 25ms
  for(int i = 0; i < x; i++){

	data.time = sigTime;
    data.voltage = getPressure();
    bloodOutput.push_back(data);

    delay(25);
    sigTime = sigTime + 0.025;
  }
}

/***********************************************************************************************************
    Function: getPressure()
    Description: Reads pressure from the pressure transducer pin
    Preconditions: None
    Postconditions: Pressure is read
************************************************************************************************************/
double getPressure()
{
  // reads raw data from the ADC
  rawSensorVal = analogRead(pVIn);
  // converts to make it fit within 3.3V
  divSensorVal = (rawSensorVal * (3300.0 / 4095.0));
  
  // converts it back so we can read the values
  double Vout = (rawSensorVal / 4095.0) * 3300.0;
  
  return Vout;
}

/***********************************************************************************************************
    Function: openValve()
    Description: Sets the valve switch PWM to 0/4095 which is open
    Preconditions: None
    Postconditions: Valve is open
************************************************************************************************************/
void openValve()
{
  ledcWrite(valveSwitch, 0);
}

/***********************************************************************************************************
    Function: closeValve()
    Description: Sets the valve switch PWM to 4096/4096 which is closed
    Preconditions: None
    Postconditions: Valve is closed
************************************************************************************************************/
void closeValve()
{
  ledcWrite(valveSwitch, 4096);
}

/***********************************************************************************************************
    Function: pumpOn()
    Description: Turns the pump on
    Preconditions: None
    Postconditions: Pump is turned on
************************************************************************************************************/
void pumpOn()
{
  digitalWrite(motor_B1, HIGH);
  digitalWrite(motor_A1, LOW);
}

/***********************************************************************************************************
    Function: pumpOff()
    Description: Turns the pump off
    Preconditions: None
    Postconditions: Pump is turned off
************************************************************************************************************/
void pumpOff()
{
  digitalWrite(motor_A1, LOW);
  digitalWrite(motor_B1, LOW);
}
