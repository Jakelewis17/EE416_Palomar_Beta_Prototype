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
 * Description: The slave header file which includes all necessary libraries,  *
 *              data stuctures, and function definitions                       *
 *                                                                             *
 *                                                                             *
 ******************************************************************************/

/* Guard Code */
#ifndef PATIENT_MONITOR_H
#define PATIENT_MONITOR_H

/* Libraries */
#include <Arduino.h>
#include <SPI.h>
#include "stdio.h"
#include <Wire.h>
#include <time.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <driver/spi_master.h>
#include "esp_timer.h"

// web server pages
#include "index.h"
#include "subject1.h"
#include "error_404.h"
#include "error_405.h"

//define pages
#define INDEX_PAGE 0
#define SUBJECT1_PAGE 1

//define error pages
#define ERROR_404_PAGE -1
#define ERROR_405_PAGE -2

#define SLAVE_ADDRESS 127

//define ECG resolution
#define ECG_SAMPLES 1000
#define ECG_CORRELATION_LENGTH 20
#define ECG_HEARTRATE_SAMPLES 5

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

/***************************************** Function definitions ********************************************/

/***********************************************************************************************************
    Function: connectserver()
    Description: Connects ESP32 to web server via WiFi
    Preconditions: None
    Postconditions: Server is connected
************************************************************************************************************/
void connectserver();

/***********************************************************************************************************
    Function: receiveEvent()
    Description: Handler when slave ESP receives data from master ESP. Checks the first byte for a 0 or 1
    Parameters: How many bytes are being received from master
    Preconditions: None
    Postconditions: None
************************************************************************************************************/
void receiveEvent(int howMany);

/***********************************************************************************************************
    Function: requestEvent()
    Description: Handler when master ESP requests data from slave. Sends the ECG data to the master
    Preconditions: None
    Postconditions: None
************************************************************************************************************/
void requestEvent();

/***********************************************************************************************************
    Function: init_ECG()
    Description: Initializes SPI bus and adds SPI device
    Preconditions: None
    Postconditions: ECG system is initialized
************************************************************************************************************/
void init_ECG();

/***********************************************************************************************************
    Function: ECG_Measurement()
    Description: Reads ECG data via Slave's ADC and performs cross-correlation algorithm
    Preconditions: SPI bus must be initialized and SPI device must be added
    Postconditions: ECG data stored in Patientdata struct
************************************************************************************************************/
void ECG_Measurement();

/***********************************************************************************************************
    Function: send_to_webserver()
    Description: Reads data via I2C from master and sends to web server over WiFi
    Preconditions: ECG leads must be plugged in to get accurate data
    Postconditions: Data viewable on Blynk app, Patientdata struct updated with correct data
************************************************************************************************************/
void send_to_webserver();

/***********************************************************************************************************
    Function: handleVariables()
    Description: Updates web server with new data received from master
    Preconditions: Data request from master must be initialized first
    Postconditions: Web server is updated
************************************************************************************************************/
void handleVariables();

/***********************************************************************************************************
    Function: generate_id_number()
    Description: Generates random ID number between 0 - 9999 to simulate real life 
    Preconditions: None
    Postconditions: None
************************************************************************************************************/
void generate_id_number();


#endif