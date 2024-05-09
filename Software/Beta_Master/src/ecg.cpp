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
 * File: ecg.cpp                                                               *
 *                                                                             *
 * Description: Contains code to communicate with the slave ESP32 over I2C to  *
 *              start an ECG measurement.                                      *                   
 *                                                                             *
 *                                                                             *
 ******************************************************************************/

#include "patient_monitor.h"

/* External global variables */
extern int what_press;
extern int ecg_control_value;
extern patientdata Patientdata;

int_arr int_u;

/* Global variables */
int BPM = 0;
int beat_old = 0;
int ecg_reading = 0;
float beats[15] = { 0 };  // Used to calculate average BPM
int beatIndex = 0;
float threshold = 130;  //Threshold at which BPM calculation occurs
boolean belowThreshold = true;
int end_measurment = 0;
int starttime_ecg = 0, endtime_ecg = 0;
int ecg_data[25] = { 0 };
int ecg_avg = 0, ecg_array_val = 0, ecg_temp = 0, ecg_total = 0;
int ecg_index = 0;
int ecg_buffer[350] = { 0 };
int ECG_index = 0;

/* Extern Blynk timer */
extern BlynkTimer timer;
extern BlynkWifi Blynk;

/* Define timer for sending data to app */
void ECG_timer() 
{
  Blynk.virtualWrite(V53, Patientdata.ECG[ECG_index]); //uncomment this
  ECG_index++; 
}

/***********************************************************************************************************
    Function: read_ecg()
    Description: Tells slave to start ECG measurement, requests ECG data from slave, sends data to Blynk app
    Preconditions: ECG leads must be plugged in to get accurate data
    Postconditions: Data viewable on Blynk app, Patientdata struct updated with correct data
************************************************************************************************************/
void read_ecg()
{
  what_press = 0;
 
  Blynk.virtualWrite(V56, "Measurement in Progress");  
  
  //set interval to update app every 10th of a second
  timer.setInterval(1000L, ECG_timer); 

  byte x = 1;
  byte RxByte;

  //send flag to slave to start ECG measurement
  Wire.begin(slaveSDA, slaveSCL);
  Wire.beginTransmission(127); //slave on address 127
  Wire.write(x); // 1 indicates ECG measurement
  Wire.endTransmission();   
  
  while(int_u.intvalue != -1) //request data until -1 flag is hit
 {
   Wire.requestFrom(127, 4); //request 4 bytes from address 127
   int i = 0;
   while(Wire.available() && i < 4)
   {
     int_u.intbytes[i] = Wire.read();
     i++;
   }
   //store in Patientdata struct
   Patientdata.ECG[ECG_index] = int_u.intvalue;
   ECG_index++;
  
 }

  ECG_index = 0; //reset index

  while(ECG_index < 1000) //run until all data has been sent
  {
    timer.run(); //run Blynk timer and send data
    Blynk.run();

    if(ecg_control_value == 0) //check to see if measurement terminated early
    {
      Blynk.virtualWrite(V50, 0); //reset ECG measurement button
      Blynk.virtualWrite(V56, "Previous Measurement Was Invalid");
      Patientdata.ECG_invalid = 1; //set invalid flag
      break;
    }

  }

  Blynk.virtualWrite(V50, 0); //reset ECG measurement button
  Blynk.virtualWrite(V56, "Measurement Complete, View Server for Details or Measure Again");  //ecg

  return;

}
