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
 * File: blood_pressure.cpp                                                    *
 *                                                                             *
 * Description: A patient monitor measuring the three most important           *
 *              physilogical parameters: blood oxygen, ECG, and blood pressure *   
 *                                                                             *
 *                                                                             *
 ******************************************************************************/


#include "patient_monitor.h"

extern int what_press;

extern patientdata Patientdata;

/* Define rotary encoder button */
//extern BfButton rotary_sw;

/* Invoke display objects from TFT library */
//extern TFT_eSPI tft;  
//extern TFT_eSprite ecg;
//extern TFT_eSprite background;
//extern TFT_eSprite title;
//extern TFT_eSprite bp_display;

float pressureVolt = 100;

extern BlynkWifi Blynk;
extern BlynkTimer timer;

int starttime_bp = 0, endtime_bp = 0;
extern int bp_control_value;
char temp_bp_number[] = "120/80 ";

void BP_timer() 
{
  Blynk.virtualWrite(V55, temp_bp_number);  
}

void read_bp()
{
  what_press = 0;
  //tft.fillScreen(TFT_BLACK);

  //display title
  //title.fillSprite(TFT_BLACK);
  //title.setTextColor(TFT_WHITE, TFT_BLACK);
  //title.drawString("BP", 0, 0);
  //title.pushSprite(90, 30);

  //display instructions
  //tft.drawString("Click once to start", 0, 80, 4);
  //tft.drawString("Click twice to return", 0, 120, 4);

  timer.setInterval(1000L, BP_timer); 
  for(int i = 0; i < 9; i++)
  {
    timer.run();
    Blynk.virtualWrite(V58, "Measurement in Progress");  //ecg
  }
  bp_control_value = 0;
  Blynk.virtualWrite(V52, bp_control_value);
  Blynk.virtualWrite(V58, "Measurement Complete, View Server for Details or Measure Again");  //ecg
  //bp_measurement();
  

/*
  for(;;) //infinite polling loop for switch input
  {
    Blynk.run();
    rotary_sw.read();
    if(what_press == 1)
    {
      //single press - jump to BP measurement function
      bp_measurement();
      what_press = 0;
      tft.fillScreen(TFT_BLACK);
      break;
    }
    else if(what_press == 2)
    {
      //double press - go backto seleciton screen
      tft.fillScreen(TFT_BLACK);
      break;
    }
    
    what_press = 0;
  }
  */
}

void bp_measurement()
{

  //tft.fillScreen(TFT_BLUE);
  //title.setTextColor(TFT_WHITE, TFT_BLUE);
  // Set air pump to stop
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);

  // initialize valve as closed
  digitalWrite(valveSwitch, LOW);
  what_press = 0;

  starttime_bp = millis();
  endtime_bp = starttime_bp;
  

  cycleBPSystem();
  /*
  for(;;)
  {

    //display title
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.drawString("Press to start... ", 40, 60, 4);
    //tft.pushSprite(40, 60);

    rotary_sw.read();
    if(what_press == 1)
    {
      //single press runs cycle function
      tft.fillScreen(TFT_BLUE);
      cycleBPSystem();
      what_press = 0;
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
  }
  */


}

void cycleBPSystem()
{
  closeValve();
  runPump30s();
  openValve();
  //delay(30000);
  //closeValve();
  //tft.fillScreen(TFT_BLUE);
}

void openValve()
{
  Serial.println("Valve open");
  digitalWrite(valveSwitch, HIGH);
}

void closeValve()
{
  Serial.println("Valve closed");
  digitalWrite(valveSwitch, LOW);
}

void runPump30s()
{
  Serial.println("Running Pump");
  pressureVolt = 100;
  pumpOn();

  for(int i = 0; i < 12; i++)
  {

    if(pressureVolt < 32)
    {
      break;
    }
    
    delay(2000);
    int sensorVal = analogRead(pVIn);
    pressureVolt = sensorVal * (12.0 / 1023.0);
    displayBP(pressureVolt);

    Serial.print("Pressure voltage: "); 
    Serial.println(pressureVolt);

    if(pressureVolt < 33)
    {
      break;
    }
 
  }
  stopPump();
}

void pumpOn()
{
  digitalWrite(A1, HIGH);
  digitalWrite(A2, LOW);
}

void stopPump()
{
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
}

void displayBP(float pressure)
{
  //display BP voltage
  //tft.drawString("Pressure Voltage: ", 30, 60, 4);
  //bp_display.drawString(String(pressure), 0, 0, 7);
  //bp_display.pushSprite(50, 150);
}
