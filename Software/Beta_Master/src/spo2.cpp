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
 * File: spo2.cpp                                                              *
 *                                                                             *
 * Description: Contains code to configure the SpO2 system, acquire data from  *
 *              it, and send it to the app                                     *              
 *                                                                             *
 *                                                                             *
 * Note: Got some code to set up the Sparkfun board from their website:        *
 *       https://www.sparkfun.com/products/15219                               *               
 ******************************************************************************/

#include "patient_monitor.h"

/* external variables */
extern int what_press;
extern int spo2_control_value;
extern patientdata Patientdata;
extern BlynkWifi Blynk;
extern BlynkTimer timer;

/* global variables */
int starttime_spo2 = 0, endtime_spo2 = 0;
int spo2_data[20] = { 0 };
int HR_data[20] = { 0 };
int avg_spo2 = 0;
int avg_HR = 0;
char result[10];

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin); 
bioData body;  

void SpO2_timer() 
{
  //concatenate result into a string
  sprintf(result, "%i", avg_spo2);

  //Write to virtual pin 54 (SpO2 data)
  if(avg_spo2 < 85)
  {
    //write ellipses if no data ready yet
    Blynk.virtualWrite(V54, "...");  
  }
  else
  {
    //write correct data to app
    Blynk.virtualWrite(V54, result);  
  }
}

/***********************************************************************************************************
    Function: read_spo2()
    Description: Sets up Blynk timer and branches to spo2 measurement function
    Preconditions: None
    Postconditions: SpO2 Blynk timer set up
************************************************************************************************************/
void read_spo2()
{
  what_press = 0;

  timer.setInterval(1000L, SpO2_timer); //set up Blynk timer
  spo2_measurment();

}


/***********************************************************************************************************
    Function: spo2_measurement()
    Description: Configures Sparkfun SpO2 board, reads data for 30 secs, calculates SpO2 and HR every second,
                 displays SpO2 reading on app, and stores data in Patientdata struct
    Preconditions: Finger must be on Sparkfun board with constant pressure to get readings
    Postconditions: Data viewable on Blynk app, Patientdata struct updated 
************************************************************************************************************/
void spo2_measurment()
{

  /* The following chunk of code was taken from Sparkfun: https://www.sparkfun.com/products/15219 
     This is configuring I2C to communicate with the sensor*/
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

  // Set sample rate per second
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
  // it's being configured this delay will give some time for the data to catch up
  Serial.println("Loading up the buffer with data....");
  
  delay(500);

  int finger_detect = 0; // temp variable

  int spo2_index = 0;
  starttime_spo2 = millis();
  endtime_spo2 = starttime_spo2;
    
    //Continuously taking samples from MAX30101.  Heart rate and SpO2 are calculated every 1 second. Run for 30 secs
    while((endtime_spo2 - starttime_spo2) <= 30000)
    {
      Blynk.virtualWrite(V57, "Measurement in Progress");  
      timer.run(); //run Blynk timers

      //some code taken from SparkFun
      //Information from the readBpm function will be saved to our "body" variable
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

      if (finger_detect == 3) //if finger detected
      {  
        calculateSpO2(spo2_index);
        calculateHR(spo2_index);  
        spo2_index++;  //update index only if finger_detected
      }
      else if (finger_detect == 0) {
        Blynk.virtualWrite(V57, "No Finger Detected"); 
      }

    //terminate measurement before done
    Blynk.run();
    if(spo2_control_value == 0) //terminate measurement
    {
      Blynk.virtualWrite(V51, 0); //reset ECG measurement button
      Blynk.virtualWrite(V57, "Pre