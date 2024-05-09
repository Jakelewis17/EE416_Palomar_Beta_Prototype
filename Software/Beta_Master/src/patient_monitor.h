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
    Preconditions: ECG leads 