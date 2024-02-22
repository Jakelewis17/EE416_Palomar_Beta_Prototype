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

/* Define rotary encoder button */
extern BfButton rotary_sw;

/* Invoke display objects from TFT library */
extern TFT_eSPI tft;  
extern TFT_eSprite ecg;
extern TFT_eSprite background;
extern TFT_eSprite title;
extern TFT_eSprite digit_box;
extern TFT_eSprite hr_display;
extern TFT_eSprite spo2_display;

extern BlynkWifi Blynk;
extern BlynkTimer timer;

int starttime_spo2 = 0, endtime_spo2 = 0;
int spo2_data[20] = { 0 };
int avg_spo2 = 0;

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin); 
bioData body;  //This is a type (int, byte, long, etc.)

void SpO2_timer() 
{
  //Write to virtual pin 54 (SpO2 data)
  if(avg_spo2 < 85)
  {
    Blynk.virtualWrite(V54, "...");  
  }
  else
  {
    Blynk.virtualWrite(V54, avg_spo2);  
  }
  
}

void read_spo2()
{
  what_press = 0;
  tft.fillScreen(TFT_BLACK);

  //display title
  title.fillSprite(TFT_BLACK);
  title.setTextColor(TFT_WHITE, TFT_BLACK);
  title.drawString("SPO2", 0, 0);
  title.pushSprite(70, 30);

  //display instructions
  tft.drawString("Click once to start", 0, 80, 4);
  tft.drawString("Click twice to return", 0, 120, 4);

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
  int error = bioHub.configBpm(MODE_ONE); // Configuring just the BPM settings. 
  if(error == 0){ // Zero errors!
    Serial.println("Sensor configured.");
  }
  else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Data lags a bit behind the sensor, if your finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up. 
  Serial.println("Loading up the buffer with data....");
 
  //display "calibrating" screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(&Dialog_plain_35); //custom font
  tft.drawString("Calibrating...: ", 15, 115);
  
  delay(500);


  tft.fillScreen(TFT_BLUE);
  title.fillSprite(TFT_BLUE);
  title.setTextColor(TFT_WHITE, TFT_BLUE);
  title.drawString("SPO2: ", 0, 0);
  title.pushSprite(70, 60);

  digit_box.setTextColor(TFT_BLACK, TFT_BLUE);
  digit_box.fillSprite(TFT_BLUE);

  int finger_detect = 0; // temp variable

  //after loading display
  tft.fillScreen(TFT_BLUE);
  spo2_display.drawString(String(body.oxygen), 0, 0, 7);
  spo2_display.pushSprite(80, 150);
  title.setTextColor(TFT_WHITE, TFT_BLUE);
  title.pushSprite(70, 60);
  digit_box.pushSprite(143, 150);

  int spo2_index = 0;
  starttime_spo2 = millis();
  endtime_spo2 = starttime_spo2;
    
    //Continuously taking samples from MAX30101.  Heart rate and SpO2 are calculated every 1 second
    while((endtime_spo2 - starttime_spo2) <= 20000)
    {
      Blynk.virtualWrite(V57, "Measurement in Progress");  
      timer.run(); //run Blynk timer

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

      // Slow it down or your heart rate will go up trying to keep up
      // with the flow of numbers
      delay(250);

      //display_spo2(finger_detect);
      if (finger_detect == 3) 
      {  
        calculateSpO2(spo2_index);
        display_spo2(3);      
        spo2_index++;  //update index only if finger_detected
      }
      else if (finger_detect == 0) {
        display_spo2(0);
        Blynk.virtualWrite(V57, "No Finger Detected"); 
      }

      //check for long press to go back to menu
      rotary_sw.read();
      if(what_press == 3)
    {
      //long press terminates the loop
      what_press = 0;
      tft.fillScreen(TFT_WHITE);
      break;
    }

    //terminate measurement before done
    Blynk.run();
    if(spo2_control_value == 0) //terminate measurement
    {
      //long press terminates the loop
      what_press = 0;
      tft.fillScreen(TFT_WHITE);
      Blynk.virtualWrite(V51, 0); //reset ECG measurement button
      Blynk.virtualWrite(V57, "Measurement Complete, View Server for Details or Measure Again");
      break;
    }
   
    endtime_spo2 = millis();
  }

  Blynk.virtualWrite(V51, 0); //reset spo2 measurement button
  Blynk.virtualWrite(V57, "Measurement Complete, View Server for Details or Measure Again");  

}


void display_spo2(int finger_detect)
{

  if(finger_detect == 0)
  {
    //no finger detected - display 0
    spo2_display.drawString("000", 0, 0, 7);
    spo2_display.pushSprite(80, 150);
  }
  else
  {  
    //display average SPO2
    spo2_display.drawString(String(body.oxygen), 0, 0, 7);
    spo2_display.pushSprite(80, 150);

    if(body.oxygen < 100)
    {
      //fix extra digit issue
      digit_box.pushSprite(143, 150);
    }      
  }


}

void calculateSpO2(int index)
{
  int spo2_temp = 0, spo2_total = 0;

  //update value in