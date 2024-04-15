/*******************************************************************************
 * Programmers: Jake Lewis, Zachary Harrington, Nicholas Gerth, Matthew Stavig *                                                      
 * Class: EE415 - Product Design Management                                    *
 * Sponsoring Company: Philips                                                 *
 * Industry Mentor: Scott Schweizer                                            *
 * Faculty Mentor: Mohammad Torabi Konjin                                      *
 *                                                                             *
 *                          Patient Monitor Project                            *
 *                                                                             *
 * Date: 4/15/2024                                                             *
 * File: patient_monitor.h                                                     *
 *                                                                             *
 * Description: A patient monitor measuring the three most important           *
 *              physilogical parameters: blood oxygen, ECG, and blood pressure *   
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

void connectserver();
void receiveData();
void receiveEvent(int howMany);
void sendDataToSlave(patientdata* data);
void init_ECG();
void ECG_Measurement();
void send_to_webserver();

void connectserver();
void handleVariables();
void generate_id_number();


#endif