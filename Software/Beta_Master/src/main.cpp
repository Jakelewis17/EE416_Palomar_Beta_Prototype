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
 * File: main.cpp                                                              *
 *                                                                             *
 * Description: Where the program idles when nothing is happening. Branches    *
 *              to seperate measurement functions based on user input on app   *                                                  
 *                                                                             *
 *                                                                             *
 ******************************************************************************/

#include "patient_monitor.h"

/* Global variables */
int what_press = 0;
int what_param = -1;
int ecg_control_value = -1;
int spo2_control_value = -1;
int bp_control_value = -1;
uint32_t tsLastReport = 0;
char CurrentUser[] = " ";

patientdata Patientdata;


/* Blynk Definitions */
static WiFiClient _blynkWifiClient;
static BlynkEsp32Client _blynkTransport(_blynkWifiClient);
BlynkWifi Blynk(_blynkTransport);
BlynkTimer timer;


BLYNK_WRITE(V50) //ECG Control
{   
  ecg_control_value = param.asInt(); // Get value as integer
  Serial.print(ecg_control_value);
  what_param = 1;
}


BLYNK_WRITE(V51) //SpO2 Control
{   
  spo2_control_value = param.asInt(); // Get value as integer
  what_param = 2;
}

BLYNK_WRITE(V52) //BP Control
{   
  bp_control_value = param.asInt(); // Get value as integer
  what_param = 3;
}

BLYNK_WRITE(V60) //Input user 
{   
  strcpy(CurrentUser, param.asString());
}

BLYNK_WRITE(V61) //User Enter
{   
  Blynk.virtualWrite(V59, CurrentUser);  //send CurrentUser to app
}

BLYNK_WRITE(V62) //User Enter
{   
  int pinValue = param.asInt(); // assigning incoming value from pin V62 to a variable
  
  if (pinValue == 1) //debounces button
  {
     sendData();
  } 
}


/* Setup Function*/
void setup() {
  
  Serial.begin(115200);
  Serial.println("Hello from startup");

  // WiFi (hotspot) credentials.
  char ssid[] = "Grog ";
  char pass[] = "beansmyguy";

  //Connect to Blynk app
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  //setup pins 
  pinMode(PinSW, INPUT_PULLUP);
  pinMode(PinCLK, INPUT);
  pinMode(PinDT, INPUT);
  pinMode(PIN_GPIO, OUTPUT);
  pinMode(PIN_SPO2_RST, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(valveSwitch, OUTPUT);

  //send status of "Idle" for each measurementto app
  Blynk.virtualWrite(V56, "Idle");  //ecg
  Blynk.virtualWrite(V57, "Idle");  //spo2
  Blynk.virtualWrite(V58, "Idle");  //bp

  delay(500);
}

/* Infinite Loop Function */
void loop() {

  Blynk.run();

  //app control
  if(what_param == 1)
  {
    //enter ecg data collection
    read_ecg();
    what_param = -1;
  }
  else if(what_param == 2)
  {
    //enter SPO2 data collection
    read_spo2();
    what_param = -1;
  }
  else if(what_param == 3)
  {
    //enter blood pressure data collection
    read_bp();
    what_param = -1;
  }
  else{
    what_param = -1;
  }
  
}

/***********************************************************************************************************
    Function: sendData()
    Description: Sends the data over I2C to the slave ESP32. Resets Patientdata struct after sending
    Preconditions: There must be data to send
    Postconditions: Data in struct is reset 
************************************************************************************************************/
void sendData()
{
  //connect to slave ESP
  Wire.begin(slaveSDA, slaveSCL);

  //get time
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  sprintf(Patientdata.date, "%d-%02d", tm.tm_year + 1954, tm.tm_mon + 4);

  //fake data for testing
  Patientdata.Spo2 = 98;
  Patientdata.SpO2_invalid = 0;
  strcpy(Patientdata.BP, "128/90\0");
  Patientdata.BP_invalid = 1;
  Patientdata.Heartrate = 135;
  Patientdata.ECG_invalid = 69;

  //send data via I2C to other esp (print values for now)
  Serial.print("Spo2: ");
  Serial.println(Patientdata.Spo2);
  Serial.print("Spo2 Valid? ");
  Serial.println(Patientdata.SpO2_invalid);
  Serial.print("BP: ");
  Serial.println(Patientdata.BP);
  Serial.print("BP Valid? ");
  Serial.println(Patientdata.BP_invalid);
  Serial.print("HR: ");
  Serial.println(Patientdata.Heartrate);
  Serial.print("Date: ");
  Serial.println(Patientdata.date);
  Serial.print("ECG: ");


  Wire.beginTransmission(127);
  Wire.write(0); // 0 indicates patient data is incoming
  Wire.write(Patientdata.Spo2);
  Wire.write(Patientdata.SpO2_invalid);
  Wire.write(Patientdata.BP);
  Wire.write(Patientdata.BP_invalid);
  Wire.write(Patientdata.Heartrate);
  Wire.write(Patientdata.date);
  int check = Wire.endTransmission(); //endTransmission sends the queued data
  Serial.print("End Transmission Code: ");
  Serial.println(check);

  //reset all values back to 0
  Patientdata.Spo2 = 0;
  Patientdata.SpO2_invalid = 0;
  Patientdata.BP_invalid = 0;
  Patientdata.ECG_invalid = 0;
  Patientdata.Heartrate = 0;
  strcpy(Patientdata.BP, " ");
  strcpy(Patientdata.date, " ");

  //reset ECG data back to 0
  for(int i = 0; i < 1000; i++)
  {
    Serial.print(Patientdata.ECG[i]);
    Patientdata.ECG[i] = 0;
  }
}
