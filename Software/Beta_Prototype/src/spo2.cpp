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
 * File: spo2.cpp                                                              *
 *                                                                             *
 * Description: A patient monitor measuring the three most important           *
 *              physilogical parameters: blood oxygen, ECG, and blood pressure *   
 *                                                                             *
 *                                                                             *
 ******************************************************************************/

#include "patient_monitor.h"

extern int what_press;
extern int spo2_control_value;

extern patientdata Patientdata;

extern BlynkWifi Blynk;
extern BlynkTimer timer;

int starttime_spo2 = 0, endtime_spo2 = 0;
int spo2_data[20] = { 0 };
int HR_data[20] = { 0 };
int avg_spo2 = 0;
int avg_HR = 0;
char result[10];

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin); 
bioData body;  //This is a type (int, byte, long, etc.)

void SpO2_timer() 
{

  sprintf(result, "%i", avg_spo2);

  //Write to virtual pin 54 (SpO2 data)
  if(avg_spo2 < 85)
  {
    Blynk.virtualWrite(V54, "...");  
  }
  else
  {
    Blynk.virtualWrite(V54, result);  
  }
}

void read_spo2()
{
  what_press = 0;

  timer.setInterval(1000L, SpO2_timer); 
  spo2_measurment();

}



void spo2_measurment()
{

  // Taken from SparkFun
  Wire.begin();
  int result = bioHub.begin();
  if (result == 0) // Zero errors!
    Serial.println("Sensor started!");
  else
    Serial.println("Could not communicate with the sensor!");
 
  Serial.println("Configuring Sensor...."); 
  int error = bioHub.configBpm(MODE_TWO); // Configuring just the BPM settings. 
  if(error == 0){ // Zero errors!
    Serial.println("Sensor configured.");
  }
  else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }


  int width = 411; 
  int samples = 400; 
  int pulseWidthVal;
  int sampleVal;

  //set up pulse width and sample length
  error = bioHub.setPulseWidth(width);
  if (!error){
    Serial.println("Pulse Width Set.");
    }
  else {
    Serial.println("Could not set Pulse Width.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Check that the pulse width was set. 
  pulseWidthVal = bioHub.readPulseWidth();
  Serial.print("Pulse Width: ");
  Serial.println(pulseWidthVal);

  // Set sample rate per second. Remember that not every sample rate is
  // available with every pulse width. Check hookup guide for more information.  
  error = bioHub.setSampleRate(samples);
  if (!error){
      Serial.println("Sample Rate Set.");
  }
  else {
      Serial.println("Could not set Sample Rate!");
      Serial.print("Error: "); 
      Serial.println(error); 
  }

  // Check sample rate.
  sampleVal = bioHub.readSampleRate();
  Serial.print("Sample rate is set to: ");
  Serial.println(sampleVal); 

  // Data lags a bit behind the sensor, if your finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up. 
  Serial.println("Loading up the buffer with data....");
  
  delay(500);

  int finger_detect = 0; // temp variable

  int spo2_index = 0;
  starttime_spo2 = millis();
  endtime_spo2 = starttime_spo2;
    
    //Continuously taking samples from MAX30101.  Heart rate and SpO2 are calculated every 1 second
    while((endtime_spo2 - starttime_spo2) <= 30000)
    {
      Blynk.virtualWrite(V57, "Measurement in Progress");  
      timer.run(); //run Blynk timers

      //some code taken from SparkFun
      //Information from the readBpm function will be saved to our "body"
      //variable.
      body = bioHub.readBpm();
      
      finger_detect = body.status; // 0 = no finger, 3 = finger detected;

      
      Serial.print("Heartrate: ");
      Serial.println(body.heartRate); 
      Serial.print("Confidence: ");
      Serial.println(body.confidence); 
      Serial.print("Oxygen: ");
      Serial.println(body.oxygen);
      Serial.print("Status: ");
      Serial.println(body.status);
      Serial.print("ExtStatus: ");
      Serial.println(body.extStatus);

      // Slow it down or your heart rate will go up trying to keep up
      // with the flow of numbers
      delay(250);

      //display_spo2(finger_detect);
      if (finger_detect == 3) 
      {  
        calculateSpO2(spo2_index);
        calculateHR(spo2_index);  
        spo2_index++;  //update index only if finger_detected
      }
      else if (finger_detect == 0) {
        Blynk.virtualWrite(V57, "No Finger Detected"); 
      }

      //check for long press to go back to menu
      //rotary_sw.read();
    if(what_press == 3)
    {
      //long press terminates the loop
      what_press = 0;
      //tft.fillScreen(TFT_WHITE);
      break;
    }

    //terminate measurement before done
    Blynk.run();
    if(spo2_control_value == 0) //terminate measurement
    {
      //long press terminates the loop
      what_press = 0;
      //tft.fillScreen(TFT_WHITE);
      Blynk.virtualWrite(V51, 0); //reset ECG measurement button
      Blynk.virtualWrite(V57, "Previous Measurement Was Invalid");
      Patientdata.SpO2_invalid = 1; //set invalid flag
      
      break;
    }
   
    endtime_spo2 = millis();
  }

  Blynk.virtualWrite(V51, 0); //reset spo2 measurement button
  Blynk.virtualWrite(V57, "Measurement Complete, View Server for Details or Measure Again");  

}

void calculateSpO2(int index)
{
  int spo2_temp = 0, spo2_total = 0;

  //update value in array
  int spo2_array_val = index % 20;
  spo2_data[spo2_array_val] = body.oxygen;

  //calulate average
  for(int i = 0; i < 20; i++)
  {
    spo2_temp = spo2_data[i];
    spo2_total += spo2_temp;
  }
  Serial.println(avg_spo2);
  avg_spo2 = spo2_total / 20;
  Patientdata.Spo2 = avg_spo2;

}

void calculateHR(int index)
{
  int HR_temp = 0, HR_total = 0;

  int HR_array_val = index % 20;
  HR_data[HR_array_val] = body.heartRate;

  //calulate average
  for(int i = 0; i < 20; i++)
  {
    HR_temp = HR_data[i];
    HR_total += HR_temp;
  }
  avg_HR = HR_total / 20;
  Patientdata.Heartrate = avg_HR;
}
