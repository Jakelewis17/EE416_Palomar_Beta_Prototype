/*******************************************************************************
 * Programmers: Jake Lewis, Zachary Harrington, Nicholas Gerth, Matthew Stavig *                                                      
 * Class: EE415 - Product Design Management                                    *
 * Sponsoring Company: Philips                                                 *
 * Industry Mentor: Scott Schweizer                                            *
 * Faculty Mentor: Mohammad Torabi Konjin                                      *
 *                                                                             *
 *                          Patient Monitor Project                            *
 *                                                                             *
 * Date: 11/23/2023                                                            *
 * File: header.h                                                              *
 *                                                                             *
 * Description: A patient monitor measuring the three most important           *
 *              physilogical parameters: blood oxygen, ECG, and blood pressure *   
 *                                                                             *
 *                                                                             *
 ******************************************************************************/

/* Guard Code */
#ifndef PATIENT_MONITOR_H
#define PATIENT_MONITOR_H

// #define BLYNK_TEMPLATE_ID "TMPL23Jqs1MZC"
// #define BLYNK_TEMPLATE_NAME "Quickstart Template"
// #define BLYNK_AUTH_TOKEN "-cOj3cjEfRULXZrwvM81BhWUClWXVW2i"

/* Libraries */
#include <Arduino.h>
#include <SPI.h>
#include "stdio.h"
#include <Wire.h>
#include <time.h>
//#include "MAX30105.h"
//#include "spo2_algorithm.h"
#include <WiFi.h>
#include <WiFiClient.h>
//#include <BlynkSimpleEsp32.h>
//#include <SparkFun_Bio_Sensor_Hub_Library.h>

// #define REPORTING_PERIOD_MS     1000
// #define MAX_BRIGHTNESS 255

// #define BLYNK_FIRMWARE_VERSION   "1.3.2"
// #define BLYNK_PRINT Serial
// #define APP_DEBUG
// #define USE_ESP32_DEV_MODULE

// #define TIMER0_INTERVAL_MS        1000
// #define TIMER0_DURATION_MS        5000
#define SLAVE_ADDRESS 127

struct patientdata{
    int Spo2;
    char BP[6];
    int Heartrate;
    char date[50];
    int ECG[1000];
    int SpO2_invalid;
    int BP_invalid;
    int ECG_invalid;
};

extern struct patientdata Patientdata;

//extern BlynkWifi Blynk;

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

/* Function definitions */
void read_ecg();
void read_spo2();
void read_bp();
void ecg_measurement();
void spo2_measurment();
void bp_measurement();
void display_spo2(int finger_detect);
void drawLine(int xPos, int analogVal);
void calculateBPM(); 
void calculateSpO2(int index);
void calculateHR(int index);

//Blood Pressure Functions
void openValve();
void closeValve();
void cycleBPSystem();
void runPump30s();
void pumpOn();
void stopPump();
void displayBP(float pressure);

void receiveData();
void receiveEvent(int howMany);
void sendDataToSlave(patientdata* data);
void deserializePatientData(patientdata& data, const uint8_t* buffer);
void ECG_Measurement();
void send_to_webserver();

#endif