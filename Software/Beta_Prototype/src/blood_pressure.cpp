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
  
  timer.setInterval(1000L, BP_timer); 
  for(int i = 0; i < 9; i++)
  {
    timer.run();
    Blynk.virtualWrite(V58, "Measurement in Progress");  //ecg
  }
  bp_control_value = 0;
  Blynk.virtualWrite(V52, bp_control_value);
  Blynk.virtualWrite(V58, "Measurement Complete, View Server for Details or Measure Again");  //ecg
  
}

void bp_measurement()
{

  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);

  // initialize valve as closed
  digitalWrite(valveSwitch, LOW);
  what_press = 0;

  starttime_bp = millis();
  endtime_bp = starttime_bp;
  

  cycleBPSystem();

}

void cycleBPSystem()
{
  closeValve();
  runPump30s();
  openValve();
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