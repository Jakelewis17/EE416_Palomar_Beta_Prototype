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
 * File: ecg.cpp                                                               *
 *                                                                             *
 * Description: A patient monitor measuring the three most important           *
 *              physilogical parameters: blood oxygen, ECG, and blood pressure *   
 *                                                                             *
 *                                                                             *
 ******************************************************************************/

#include "patient_monitor.h"

/* External global variables */
extern int what_press;
extern int ecg_control_value;

extern patientdata Patientdata;


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
  //Write to virtual pin 53 (ECG data)
  //Blynk.virtualWrite(V53, ecg_reading);  
  //Blynk.virtualWrite(V53, ecg_buffer[ecg_index]);
  Blynk.virtualWrite(V53, Patientdata.ECG[ECG_index]); //uncomment this
  //Serial.println("In ECG_Timer");
  //Serial.println(Patientdata.ECG[ECG_index]);

  ECG_index++; 
}


void read_ecg()
{
  what_press = 0;
 
  Blynk.virtualWrite(V56, "Measurement in Progress");  
  
  //set interval to update app every 10th of a second
  timer.setInterval(1000L, ECG_timer); 

  //ecg_measurement();
  byte x = 1;
  byte RxByte;
  //int RxByte;

  //send flag to slave to start ECG measurement
  Wire.begin(slaveSDA, slaveSCL);
  Serial.println("Before Transmission");
  Wire.beginTransmission(127);
  Serial.println("Before write");
  Wire.write(x); // 1 indicates ECG measurement
  /*
  for(int i = 0; i < 10; i++)
  {
    Wire.beginTransmission(127);
    Wire.write(x); // 1 indicates ECG measurement
    Wire.endTransmission();  
    x++;
  }
  */
  char test_char;
  Wire.endTransmission();   
  Serial.println("Before receive");
  Wire.onReceive(ECGreceiveEvent);
  Serial.println("After receive");
  Wire.requestFrom(127, 255);
  //while(ECG_index < 1000)
  //{
    
    while(Wire.available())
    {
      //RxByte = Wire.read();
      test_char = Wire.read();
      Serial.println(test_char);   
    }

    //Patientdata.ECG[ECG_index] = (int)RxByte; //store data into array
    //Serial.println((int)RxByte); 
        
    //ECG_index++;
  //}


  while(ECG_index < 1000); //wait for enough data to accumulate
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

void ECGreceiveEvent(int howMany)
{
  Serial.println("In ECG Receieve");
  while(0 < Wire.available()) // loop through all but the last
  {
    int ECG_data = Wire.read();    // receive byte as an integer
    Patientdata.ECG[ECG_index] = ECG_data; //store data into array
    Serial.print(ECG_data);        
    ECG_index++;
  }
  
}



void ecg_measurement()
{
  //tft.fillScreen(TFT_BLUE);
  ecg_reading = 0;
  int ecg_counter = 0;
  int x2 = 0, y2 = 0, y2_prev = 0;
  int ecg_spacing = 7;
  int xWriteIndex = 0; 
  int yWriteIndex = 0;
  int heartrate = 0, prev_hr = 0;
  int heartbeat_counter = 0, heartbeat_flag = 0;
  int initial_measurments = 0;
  int bin_count = 0, bin_avg = 0, bin_total = 0;
  int bin[20] = {0};

  int temp_reading[10] = {1500, 1400, 1500, 1200, 1100, 1900, 1300, 1500, 1500, 1500};
  
  int temp_index = 0;
  
  starttime_ecg = millis();
  endtime_ecg = starttime_ecg;

  while((endtime_ecg - starttime_ecg) <= 30000) //infinite loop getting measurement or run for 20 seconds
  {
    //timer.run(); //run Blynk timer

    Blynk.virtualWrite(V56, "Measurement in Progress");  //ecg

    //get analog input 
    //ecg_reading = analogRead(PinECG);
    ecg_reading = temp_reading[ecg_index % 10];
    ecg_buffer[ecg_index] = ecg_reading;
    Patientdata.ECG[ecg_index] = ecg_reading;

    Serial.println(ecg_reading);


    ecg_array_val = ecg_index % 25;
    ecg_data[ecg_array_val] = ecg_reading;

    //calulate average
    for(int i = 0; i < 25; i++)
    {
     ecg_temp = ecg_data[i];
     ecg_total += ecg_temp;
    }
    ecg_avg = ecg_total / 25;
    //ecg_reading = ecg_reading / 40;
    //17
    ecg_index++;

    //wait for 10 seconds to start displaying data
    if((endtime_ecg - starttime_ecg) >= 10000)
    {
      timer.run(); //run Blynk timer
    }


    ecg_counter++;

    //check for long press to go back to menu
    //rotary_sw.read();
    if(what_press == 3)
    {
      //long press terminates the loop
      what_press = 0;
      //tft.fillScreen(TFT_WHITE);
     // ITimer0.stopTimer();
      break;
    }

    //terminate measurement before done
    Blynk.run();
    if(ecg_control_value == 0) //terminate measurement
    {
      //long press terminates the loop
      what_press = 0;
      //tft.fillScreen(TFT_WHITE);
      Blynk.virtualWrite(V50, 0); //reset ECG measurement button
      Blynk.virtualWrite(V56, "Previous Measurement Was Invalid");
      Patientdata.ECG_invalid = 1; //set invalid flag
      break;
    }

    temp_index++;
    heartbeat_counter++;
    
    //detect heartbeat by checking if reading goes over a threshold
    if(ecg_reading > 130)
    {
      //detect heartbeat
      heartbeat_flag = 1;
      
    }

    //calculate heartbeat
    if(heartbeat_flag == 1)
    {
      prev_hr = heartrate;
      heartrate = 750 / heartbeat_counter;
      //heartrate = 1000 / heartbeat_counter;
      heartbeat_counter = 0;
      heartbeat_flag = 0;
    }


    if(BPM < 100)
    {
      //digit_box.pushSprite(160, 40);
    }

    // BPM calculation check
    if (heartrate > threshold && belowThreshold == true)
    {
      calculateBPM();
      belowThreshold = false;
    }
    else if(heartrate < threshold)
    {
      belowThreshold = true;
    }
    
    

    delay(70);
    

    if(initial_measurments < 50)
    {
      initial_measurments++;
    }
    

    endtime_ecg = millis();
    
  }

  //continute to run timer until all ECG data used up
  while(ecg_index < 350)
  {
    Blynk.run();
    ecg_index++;
    delay(90);
  }

  Blynk.virtualWrite(V50, 0); //reset ECG measurement button
  Blynk.virtualWrite(V56, "Measurement Complete, View Server for Details or Measure Again");  //ecg
}

void calculateBPM () 
{  
  int beat_new = millis();    // get the current millisecond
  int diff = beat_new - beat_old;    // find the time between the last two beats
  float currentBPM = 60000 / diff;    // convert to beats per minute
  beats[beatIndex] = currentBPM;  // store to array to convert the average
  float total = 0.0;
  for (int i = 0; i < 15; i++){
    total += beats[i];
  }
  BPM = int(total / 15);
  beat_old = beat_new;
  beatIndex = (beatIndex + 1) % 15;  // cycle through the array instead of using FIFO queue
  }
