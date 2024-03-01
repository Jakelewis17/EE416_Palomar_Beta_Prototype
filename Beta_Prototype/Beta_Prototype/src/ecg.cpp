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

/* Define rotary encoder button */
extern BfButton rotary_sw;

/* Invoke display objects from TFT library */
extern TFT_eSPI tft;  
extern TFT_eSprite ecg;
extern TFT_eSprite background;
extern TFT_eSprite title;
extern TFT_eSprite hr_display;
extern TFT_eSprite digit_box;

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

/* Extern Blynk timer */
extern BlynkTimer timer;

/* Define timer for sending data to app */
void ECG_timer() 
{
  //Write to virtual pin 53 (ECG data)
  Blynk.virtualWrite(V53, ecg_reading);  
}


void read_ecg()
{
  what_press = 0;
  
  //set interval to update app every half second
  timer.setInterval(500L, ECG_timer); 
  ecg_measurement();
}



void ecg_measurement()
{
  tft.fillScreen(TFT_BLUE);
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

  hr_display.setTextColor(TFT_WHITE, TFT_BLUE);

  //display heartrate
  title.fillSprite(TFT_BLUE);
  title.setTextColor(TFT_WHITE, TFT_BLUE);
  title.drawString("HR: ", 0, 0);
  title.pushSprite(30, 40);

  digit_box.setTextColor(TFT_BLACK, TFT_BLUE);
  digit_box.fillSprite(TFT_BLUE);

  int ecg_index = 0;
  starttime_ecg = millis();
  endtime_ecg = starttime_ecg;

  while((endtime_ecg - starttime_ecg) <= 30000) //infinite loop getting measurement or run for 20 seconds
  {
    //timer.run(); //run Blynk timer

    Blynk.virtualWrite(V56, "Measurement in Progress");  //ecg

    //get analog input 
    ecg_reading = analogRead(PinECG);
    Patientdata.ECG[ecg_index] = ecg_reading;

    Serial.println(ecg_reading);

    //rudimentary DSP
    /*
    if(ecg_reading < 300)
    {
      ecg_reading = ecg_avg;
    }
    else if((ecg_reading >= 300) && (ecg_reading < 500))
    {
      ecg_reading = 400;
    }
    else if((ecg_reading >= 500) && (ecg_reading < 700))
    {
      ecg_reading = 600;
    }
    else if((ecg_reading >= 700) && (ecg_reading < 900))
    {
      ecg_reading = 800;
    }
    else if((ecg_reading >= 900) && (ecg_reading < 1100))
    {
      ecg_reading = 1000;
    }
    else if((ecg_reading >= 1100) && (ecg_reading < 1300))
    {
      ecg_reading = 1200;
    }
    else if((ecg_reading >= 1300) && (ecg_reading < 1500))
    {
      ecg_reading = 1400;
    }
    else if((ecg_reading >= 1500) && (ecg_reading < 1700))
    {
      ecg_reading = 1600;
    }
    else if((ecg_reading >= 1700) && (ecg_reading < 1900))
    {
      ecg_reading = 1800;
    }
    else
    {
      ecg_reading = 2000;
    }
    */

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
    timer.run(); //run Blynk timer

    //ecg_reading = ecg_reading % 220;
    //if(temp_index == 10)
    //{
     // temp_index = 0;
    //}
    //ecg_reading = temp_reading[temp_index] / 14;

    //reset screen once it gets to edge
    if(ecg_counter > (tft.width() / ecg_spacing))
    {
      ecg_counter = 0;
      tft.fillScreen(TFT_BLUE);
      title.pushSprite(30, 40);
    }

    //if counter reset, move x back to left side of screen
    if(ecg_counter == 0)
    {
      xWriteIndex = 0;
      yWriteIndex = (tft.height() - ecg_reading)/2;
      //yWriteIndex = (double)(tft.height() - ecg_reading) / 1.5;
    }

    //if not on left side of screen, update indicies and display signal
    if(ecg_counter > 0)
    {
      //y2_prev = y2;
      x2 = ecg_counter * ecg_spacing;
      //y2 = (double)(tft.height() - ecg_reading + 30) / 1.5;
      y2 = (tft.height() - ecg_reading + 30) / 2;


      //get average for first 10 cycles first
      
      if(bin_count == 20)
      {
        bin_count = 0;
      }

      bin[bin_count] = y2;
      bin_count++;

      //get average of bin
      for(int i = 0; i < 20; i++)
      {
        bin_total = bin_total + bin[i];
      }
      bin_avg = bin_total / 20;


      if((y2 - bin_avg > 100) || (bin_avg - y2 < 100))
      {
        y2 = bin_avg;
      }
      

      tft.drawLine(xWriteIndex, yWriteIndex, x2, y2, TFT_WHITE);
      xWriteIndex = x2;
      yWriteIndex = y2;
    }

    ecg_counter++;

    //check for long press to go back to menu
    rotary_sw.read();
    if(what_press == 3)
    {
      //long press terminates the loop
      what_press = 0;
      tft.fillScreen(TFT_WHITE);
     // ITimer0.stopTimer();
      break;
    }

    //terminate measurement before done
    Blynk.run();
    if(ecg_control_value == 0) //terminate measurement
    {
      //long press terminates the loop
      what_press = 0;
      tft.fillScreen(TFT_WHITE);
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

    //logic for displaying correctly
    //if((prev_hr >= 100) && (heartrate < 100))
   // {
      //hr_display.drawString(String(0), 100, 30, 7);
      //hr_display.fillSprite(TFT_BLUE);

      //fix extra digit issue
      //digit_box.pushSprite(160, 40);

    //}

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
    
    if(initial_measurments >= 50)
    {
      hr_display.drawString(String(BPM), 0, 0, 7);
      hr_display.pushSprite(100, 30);
      digit_box.pushSprite(163, 30);
    }

    //tft.setTextColor(TFT_RED, TFT_GREEN);
    //digit_box.pushSprite(160, 40);

    //display heart rate
    //char heartrate_string[10];
    //sprintf(heartrate_string, "%02d", heartrate);
    //hr_display.drawString(String(heartrate), 0, 0, 7);
    //hr_display.drawString(heartrate_string, 0, 0, 7);
    //hr_display.pushSprite(100, 30);

    delay(70);
    

    //Serial.println("ECG: ");
    //Serial.println(ecg_reading);

    //Serial.println("HR ");
    //Serial.println(BPM);

    //Serial.println(ecg_control_value);

    if(initial_measurments < 50)
    {
      initial_measurments++;
    }
    /*
    static unsigned long lastTimer0 = 0;
	  static bool timer0Stopped         = false;

	  if (millis() - lastTimer0 > TIMER0_DURATION_MS)
	  {
	  	lastTimer0 = millis();

	  	if (timer0Stopped)
		  {
		  	Serial.print(F("Start ITimer0, millis() = "));
		  	Serial.println(millis());
		  	ITimer0.restartTimer();
		  }
		  else
	  	{
		  	Serial.print(F("Stop ITimer0, millis() = "));
		  	Serial.println(millis());
		  	ITimer0.stopTimer();
	  }

		timer0Stopped = !timer0Stopped;
    
    
	}
  */
    endtime_ecg = millis();
    
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
