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
 * File: Patient_monitor.h                                                     *
 *                                                                             *
 * Description: The master header file which includes all necessary libraries, *
 *              Blynk credential information, data stuctures, and function     *
 *              definitions                                                    *
 *                                                                             *
 *                                                                             *
 ******************************************************************************/

/* Guard Code */
#ifndef PATIENT_MONITOR_H
#define PATIENT_MONITOR_H

#define BLYNK_TEMPLATE_ID "TMPL23Jqs1MZC"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "-cOj3cjEfRULXZrwvM81BhWUClWXVW2i"

/* Libraries */
#include <Arduino.h>
#include <SPI.h>
#include "stdio.h"
#include <Wire.h>
#include <time.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SparkFun_Bio_Sensor_Hub_Library.h>

/* Blynk Definitions */
#define BLYNK_FIRMWARE_VERSION   "1.3.2"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_ESP32_DEV_MODULE
#define TIMER0_INTERVAL_MS        1000
#define TIMER0_DURATION_MS        5000

#define ECG_SAMPLES 1000

/* User defined type to store patient data */
struct patientdata{
    int Spo2;
    char BP[7];
    int Heartrate;
    char date[8];
    int ECG[ECG_SAMPLES];
    int SpO2_invalid;
    int BP_invalid;
    int ECG_invalid;
    char name[15];
};
extern struct patientdata Patientdata;

/* Union for easiesr I2C communication between master and slave */
union int_arr
{
   byte    intbytes[sizeof(int)];
   int     intvalue;   
};


/* Define IO Pins */
const int PinCLK = 27;
const int PinDT = 14;
const int PinSW = 13;
const int PinECG = 34;
const int slaveSDA = 21;
const int slaveSCL = 22;
const int PIN_GPIO = 19;
const int PIN_SPO2_RST = 18;
const int PINBP = 2;
const int A1 = 16;
const int A2 = 17;
const int valveSwitch = 18;
const int pVIn = 2;
const int resPin = 4;
const int mfioPin = 5;

/***************************************** Function definitions ********************************************/


/***********************************************************************************************************
    Function: read_ecg()
    Description: Tells slave to start ECG measurement, requests ECG data from slave, sends data to Blynk app
    Preconditions: ECG leads must be plugged in to get accurate data
    Postconditions: Data viewable on Blynk app, Patientdata struct updated with correct data
************************************************************************************************************/
void read_ecg();

/***********************************************************************************************************
    Function: read_spo2()
    Description: Sets up Blynk timer and branches to spo2 measurement function
    Preconditions: None
    Postconditions: SpO2 Blynk timer set up
************************************************************************************************************/
void read_spo2();

/***********************************************************************************************************
    Function: read_bp()
    Description: Sets up Blynk timer and branches to bp measurement function
    Preconditions: None
    Postconditions: BP Blynk timer set up
************************************************************************************************************/
void read_bp();

/***********************************************************************************************************
    Function: spo2_measurement()
    Description: Configures Sparkfun SpO2 board, reads data for 30 secs, calculates SpO2 and HR every second,
                 displays SpO2 reading on app, and stores data in Patientdata struct
    Preconditions: Finger must be on Sparkfun board with constant pressure to get readings
    Postconditions: Data viewable on Blynk app, Patientdata struct updated 
************************************************************************************************************/
void spo2_measurment();

/***********************************************************************************************************
    Function: calculateSpO2()
    Description: Updates SpO2 data array with data point and calculates the average of all data points
    Parameters: The index of the SpO2 data array
    Preconditions: Finger needs to be detected on Sparkfun board
    Postconditions: SpO2 Data array and average updated
************************************************************************************************************/
void calculateSpO2(int index);

/***********************************************************************************************************
    Function: calculateHR()
    Description: Updates HR data array with data point and calculates avg of all data points
    Parameters: The index of the HR data array
    Preconditions: Finger needs to be detected on Sparkfun board
    Postconditions: HR data array and average updated 
************************************************************************************************************/
void calculateHR(int index);

/***********************************************************************************************************
    Function: openValve()
    Description: Sets the valve switch PWM to 0/4095 which is open
    Preconditions: None
    Postconditions: Valve is open
************************************************************************************************************/
void openValve();

/***********************************************************************************************************
    Function: closeValve()
    Description: Sets the valve switch PWM to 4096/4096 which is closed
    Preconditions: None
    Postconditions: Valve is closed
************************************************************************************************************/
void closeValve();

/***********************************************************************************************************
    Function: pumpOn()
    Description: Turns the pump on
    Preconditions: None
    Postconditions: Pump is turned on
************************************************************************************************************/
void pumpOn();

/***********************************************************************************************************
    Function: pumpOff()
    Description: Turns the pump off
    Preconditions: None
    Postconditions: Pump is turned off
************************************************************************************************************/
void pumpOff();

/***********************************************************************************************************
    Function: getPressure()
    Description: Reads pressure from the pressure transducer pin
    Preconditions: None
    Postconditions: Pressure is read
************************************************************************************************************/
double getPressure();

/***********************************************************************************************************
    Function: samplePressureSig()
    Description: Samples the outgoing signal for a period of time
    Parameters: The number of seconds the pressure will be sampled for 
    Preconditions: Pump must be inflated enough to have a readable pressure
    Postconditions: The pressure of the cuff in mV
************************************************************************************************************/
void samplePressureSig(int s);

/***********************************************************************************************************
    Function: convertMVtommHG()
    Description: Converts the millivolt reading of the pressure into a mm of mercury reading
    Parameters: The millivolt reading of the pressure
    Preconditions: None
    Postconditions: The reading is converted
************************************************************************************************************/
double convertMVtommHG(double mv);

/***********************************************************************************************************
    Function: findBPPoints()
    Description: Finds the systolic and diastolic blood pressure from an input vector
    Parameters: The blood pressure data as a vector, the the window size for accuracy, and how long the 
                reading took in seconds 
    Preconditions: Data must have been sampled already to form a vector
    Postconditions: The blood pressure is returned 
************************************************************************************************************/
void findBPPoints(std::vector<bloodData> v, int windowSize, double s);

/***********************************************************************************************************
    Function: cycleBPSystem()
    Description: THe main flow of operations: Sets up pump, inflates cuff, samples the pressure, opens the 
                 valve, finds the blood pressure points, and converts them
    Preconditions: None
    Postconditions: BP points are found
************************************************************************************************************/
void cycleBPSystem();

/***********************************************************************************************************
    Function: init()
    Description: Initializes the pins and valve status
    Preconditions: None
    Postconditions: System is ready to get pressure
************************************************************************************************************/
void init();

/***********************************************************************************************************
    Function: sendData()
    Description: Sends the data over I2C to the slave ESP32. Resets Patientdata struct after sending
    Preconditions: There must be data to send
    Postconditions: Data in struct is reset 
************************************************************************************************************/
void sendData();


#endif