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
 * File: main.cpp                                                              *
 *                                                                             *
 * Description: A patient monitor measuring the three most important           *
 *              physilogical parameters: blood oxygen, ECG, and blood pressure *   
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

patientdata Patientdata;

/* Define rotary encoder button */
BfButton rotary_sw(BfButton::STANDALONE_DIGITAL, PinSW, true, LOW);

/* Invoke display objects from TFT library */
TFT_eSPI tft = TFT_eSPI();  
TFT_eSprite ecg = TFT_eSprite(&tft);
TFT_eSprite background = TFT_eSprite(&tft);
TFT_eSprite title = TFT_eSprite(&tft);
TFT_eSprite digit_box = TFT_eSprite(&tft);
TFT_eSprite hr_display = TFT_eSprite(&tft);
TFT_eSprite spo2_display = TFT_eSprite(&tft);
TFT_eSprite bp_display = TFT_eSprite(&tft);

MAX30105 particleSensor;

/* Define interrupt variables */
volatile boolean TurnDetected; //need volatile for interrupts
volatile boolean SWDetected; 

/*  Main Variable Definitions */
int PreviousCLK;
int PreviousDATA;
int LEDBrightness = 0;
int count = 0;
int displayState = 0;
int ecg_position = 0;
int title_bg_color = 0;
int bg_color = 0;
int spo2_title_bg_color = 0;
uint32_t tsLastReport = 0;
char CurrentUser[] = " ";
//char* CurrentUser;

/* Blynk Definitions */
static WiFiClient _blynkWifiClient;
static BlynkEsp32Client _blynkTransport(_blynkWifiClient);
BlynkWifi Blynk(_blynkTransport);
BlynkTimer timer;

/* Define interrupt routines for KY-040 rotary encoder */
void rotarydetect()
{
  //interrupt routine runs if CLK pin changes state
  TurnDetected = true;
}

/* Encoder press handler */
void swHandler(BfButton* btn, BfButton::press_pattern_t pattern)
{
  //update what_press variable depending on type of input from button
  switch(pattern)
  {
    case BfButton::SINGLE_PRESS:
    Serial.println("single press");
    what_press = 1;
    break;

    case BfButton::DOUBLE_PRESS:
    Serial.println("Double Press");
    what_press = 2;
    break;

    case BfButton::LONG_PRESS:
    Serial.println("Long Press");
    what_press = 3;
    break;
  }
}

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
  sendData();
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

  //get initial states of variables
  PreviousCLK = digitalRead(PinCLK);
  PreviousDATA = digitalRead(PinDT);

  //setup pins 
  pinMode(PinSW, INPUT_PULLUP);
  pinMode(PinCLK, INPUT);
  pinMode(PinDT, INPUT);
  pinMode(PIN_GPIO, OUTPUT);
  pinMode(PIN_SPO2_RST, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(valveSwitch, OUTPUT);

  //set up interrupt
  attachInterrupt(PinCLK, rotarydetect, CHANGE); 

  //Encoder button setup
  rotary_sw.onPress(swHandler)
  .onDoublePress(swHandler)
  .onPressFor(swHandler, 1000);

  /* TFT display setup */
  tft.init();
  tft.setRotation(14);
  tft.fillScreen(TFT_BLACK);
  tft.invertDisplay( true ); //invert display colors for proper displaying
  tft.setSwapBytes(true);
  tft.setTextWrap(true, true);


  /* TFT sprites setup*/
  ecg.createSprite(240,107); //create sprite for ecg waveform
  ecg.setSwapBytes(true);

  background.createSprite(220,220); //create sprite for background
  background.setSwapBytes(true);

  title.createSprite(120,90); //create sprite for physilogical parameter titles
  title.setTextColor(TFT_BLACK, TFT_WHITE);
  title.setFreeFont(&Dialog_plain_35); //custom font

  digit_box.createSprite(35,50); //create sprite for physilogical parameter titles
  digit_box.setTextColor(TFT_BLACK, TFT_WHITE);
  digit_box.setFreeFont(&Dialog_plain_35); //custom font

  hr_display.createSprite(93, 47);
  hr_display.setSwapBytes(true);
  hr_display.setTextColor(TFT_WHITE, TFT_BLUE);

  spo2_display.createSprite(93, 47);
  spo2_display.setSwapBytes(true);
  spo2_display.setTextColor(TFT_WHITE, TFT_BLUE);

  bp_display.createSprite(150, 47);
  bp_display.setSwapBytes(true);
  bp_display.setTextColor(TFT_WHITE, TFT_BLUE);

  
  tft.fillScreen(TFT_WHITE); //fill the screen with white

  //send status of "Idle" for each measurementto app
  Blynk.virtualWrite(V56, "Idle");  //ecg
  Blynk.virtualWrite(V57, "Idle");  //spo2
  Blynk.virtualWrite(V58, "Idle");  //bp

  delay(500);


}

/* Infinite Loop Function */
void loop() {

  Blynk.run();

  

if(TurnDetected) //check if rotary encoder detected an input
{
  TurnDetected = false;
  count++;
  
  //check if turn was to the left
  if(((digitalRead(PinCLK) == 0) && (digitalRead(PinDT) == 0)) || ((digitalRead(PinCLK) == 1) && (digitalRead(PinDT) == 1)))
  {
    //turn to left detected

    //update display state variable
    if(displayState == 0)
    {
      displayState = 2;
    }
    else
    {
      displayState--;
    }


  }
  else if (((digitalRead(PinCLK) == 0) && (digitalRead(PinDT) == 1)) || ((digitalRead(PinCLK) == 1) && (digitalRead(PinDT) == 0)))
  {
    //turn to right detected

     if(displayState == 2)
     {
       displayState = 0;
     }
     else
     {
       displayState++;
     }
  }

}

//Serial.print((count % 3));
/* Check what parameter should be displayed and display it */
if((count % 3) == 0)
{
  /* ECG Parameter */

  //change background color
  if(bg_color != 0)
  {
    tft.fillScreen(TFT_WHITE);
    bg_color = 0;
  }
  
  //display ECG text
  title.fillSprite(TFT_WHITE);
  title.drawString("ECG", 45, 35);

  //display ecg waveform image
  ecg.pushImage(0,0,240,107,ecg_sig);
  ecg.pushSprite(ecg_position, 133, TFT_BLACK);
  
  //Create flashing behind text
  if((ecg_position % 10) == 0)
  {
    if(title_bg_color == 0)
    {
      title.setTextColor(TFT_BLACK, TFT_WHITE);
      title_bg_color = 1;
    }
    else
    {
      title.setTextColor(TFT_BLACK, TFT_SKYBLUE);
      title_bg_color = 0;
    }
  }

  //push ECG sprite
  ecg.pushImage(0,0,240,107,ecg_sig);
  ecg.pushSprite(ecg_position,133, TFT_WHITE);

  title.pushSprite(35, 30);

}
else if((count % 3) == 1)
{
  /* Blood Pressure Paramter */

  //change background color
  if(bg_color != 1)
  {
    tft.fillScreen(TFT_BLACK);
    bg_color = 1;
  }

  //display BP text
  title.fillSprite(TFT_PINK);
  title.setTextColor(TFT_WHITE, TFT_PINK);
  title.drawString("BP", 63, 50);
  
  
  title.pushSprite(35, 40, TFT_PINK);
  rotary_sw.read();
  
}
else  
{
  /* SPO2 Parameter */

  //change background color
  if(bg_color != 2)
  {
    tft.fillScreen(TFT_RED);
    bg_color = 2;
  }

  //flashing behind text
  if(spo2_title_bg_color == 0)
  {
    title.setTextColor(TFT_WHITE, TFT_BLACK);
    spo2_title_bg_color = 1;
  }
  else
  {
    title.setTextColor(TFT_WHITE, TFT_RED);
    spo2_title_bg_color = 0;
  }

  //display BP text
  title.fillSprite(TFT_RED);
  title.drawString("SPO2", 0, 0);

  title.pushSprite(72, 170);
  rotary_sw.read();

}

  /* Detect a push of the rotary encoder and enter measurement */
  rotary_sw.read();
  if((what_press == 1) && ((count % 3) == 0))
  {
    //enter ecg data collection
    read_ecg();
  }
  else if ((what_press == 1) && ((count % 3) == 1))
  {
    //enter blood pressure data collection
    read_bp();
  }
  else if ((what_press == 1) && ((count % 3) == 2))
  {
    //enter SPO2 data collection
    read_spo2();
  }

  what_press = 0;

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

void sendData()
{
  //get time
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  sprintf(Patientdata.date, "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, 
  tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

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
  

  //reset all values back to 0
  Patientdata.Spo2 = 0;
  Patientdata.SpO2_invalid = 0;
  Patientdata.BP_invalid = 0;
  Patientdata.ECG_invalid = 0;
  Patientdata.Heartrate = 0;
  strcpy(Patientdata.BP, " ");
  strcpy(Patientdata.date, " ");

  for(int i = 0; i < 1000; i++)
  {
    Serial.print(Patientdata.ECG[i]);
    Patientdata.ECG[i] = 0;
  }
}

