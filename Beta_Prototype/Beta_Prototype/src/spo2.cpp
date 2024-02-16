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

/* SPO2 Variables */
extern uint32_t tsLastReport;

int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid
extern MAX30105 particleSensor;
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
int heartrate_data[50];
int spo2_data[50];

extern BlynkWifi Blynk;
extern BlynkTimer timer;

int starttime_spo2 = 0, endtime_spo2 = 0;

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

  spo2_measurment();

  /*
  for(;;) //infinite polling loop for switch input
  {
    rotary_sw.read();
    if(what_press == 1)
    {
      //single press - jump to SpO2 measurement function
      spo2_measurment();
      what_press = 0;
      tft.fillScreen(TFT_RED);
      break;
    }
    else if(what_press == 2)
    {
      //double press - go backto seleciton screen
      tft.fillScreen(TFT_RED);
      break;
    }
    
    what_press = 0;
  }
  */
}

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
}

void spo2_measurment()
{
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  // // Asynchronously dump heart rate and oxidation levels to the serial
  // //address: 0x55
  // Wire.begin(SDA_PIN, SCL_PIN);
  // Serial.println("Wire begin");
  // Wire.beginTransmission(0x55);
  // Serial.println("Wire begin 2");

  // digitalWrite(PIN_GPIO, HIGH);
  // digitalWrite(PIN_SPO2_RST, LOW);
  // delay(10);
  // digitalWrite(PIN_SPO2_RST, HIGH);
  // delay(1000);
  // pinMode(PIN_GPIO, INPUT_PULLUP); // To be used as an interrupt later

  // // Register a callback for the beat detection
  // pox.setOnBeatDetectedCallback(onBeatDetected);

  // // if (!pox.begin()) 
  // //   {
  // //       Serial.println("FAILED");
  // //       for(;;)
  // //       {
  // //         tft.fillScreen(TFT_BLACK);
  // //         tft.drawString("Failed", 0, 80, 4);
  // //       }
  // //       return;
  // //   } 
  // //   else 
  // //   {
  // //       Serial.println("SUCCESS");
  // //   }

  // for(;;)
  // {
    
  //   // For both, a value of 0 means "invalid"
  //   if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
  //       Serial.print("Heart rate:");
  //       Serial.print(pox.getHeartRate());
  //       Serial.print("bpm / SpO2:");
  //       Serial.print(pox.getSpO2());
  //       Serial.println("%");

  //       tsLastReport = millis();
  //   }

  //   pox.update();

  //   rotary_sw.read();
  //   if(what_press == 3)
  //   {
  //     //long press terminates the loop
  //     what_press = 0;
  //     tft.fillScreen(TFT_RED);
  //     break;
  //   }
  // }

  tft.fillScreen(TFT_BLUE);
  title.fillSprite(TFT_BLUE);
  title.setTextColor(TFT_WHITE, TFT_BLUE);
  title.drawString("SPO2: ", 0, 0);
  title.pushSprite(70, 60);

  digit_box.setTextColor(TFT_BLACK, TFT_BLUE);
  digit_box.fillSprite(TFT_BLUE);

  // title2.fillSprite(TFT_BLUE);
  // title2.setTextColor(TFT_WHITE, TFT_BLUE);
  // title2.drawString("SPO2: ", 0, 0);
  // title2.pushSprite(30, 150);

  /* Large portion of code taken from Sparkfun library */

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 not found"));
    while (1);
  }

  Serial.read();

  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
  int index = 0;
  int avg_spo2 = 0;
  int avg_hr = 0;
  int finger_detect = 0;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(&Dialog_plain_35); //custom font
  tft.drawString("Calibrating...: ", 15, 115);

  starttime_spo2 = millis();
  endtime_spo2 = starttime_spo2;

  for(;;)
  {
    bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps

  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();

    if(irBuffer[i] < 50000)
      {
        //no finger detected
        finger_detect = 0;
        //display_spo2(finger_detect);
        finger_detect = 1;
      }

    particleSensor.nextSample(); //We're finished with this sample so move to next sample

    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);


  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  
  //after loading
  tft.fillScreen(TFT_BLUE);
  spo2_display.drawString(String(spo2), 0, 0, 7);
  spo2_display.pushSprite(80, 150);
  title.setTextColor(TFT_WHITE, TFT_BLUE);
  title.pushSprite(70, 60);
  digit_box.pushSprite(143, 150);

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while((endtime_spo2 - starttime_spo2) <= 5000)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
    
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();

      if(irBuffer[i] < 50000)
      {
        //no finger detected
        finger_detect = 0;
        display_spo2(finger_detect);
        finger_detect = 1;
      }
      particleSensor.nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);

      //check for long press to go back to menu
      rotary_sw.read();
      if(what_press == 3)
      { 
        break;
      }

    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    Serial.println(spo2);
    Serial.println(heartRate);

    //display
    display_spo2(1);

    //check for long press to go back to menu
    if(what_press == 3)
    {
      //long press terminates the loop
      break;
    }
    endtime_spo2 = millis();
  }
  what_press = 3;
  //check for long press to go back to menu
  if(what_press == 3)
  {
    //long press terminates the loop
    what_press = 0;
    tft.fillScreen(TFT_WHITE);
    break;
  }

  }

}


void display_spo2(int finger_detect)
{

  if(finger_detect == 0 || spo2 == -999)
  {
    //no finger detected - display 0
    spo2_display.drawString("000", 0, 0, 7);
    spo2_display.pushSprite(80, 150);
    //digit_box.pushSprite(143, 150);
  }
  else
  {
    if(spo2 != -999) //if -999, invalid so don't display
    {
      //display average SPO2
      spo2_display.drawString(String(spo2), 0, 0, 7);
      spo2_display.pushSprite(80, 150);

      if(spo2 < 100)
      {
        //fix extra digit issue
        digit_box.pushSprite(143, 150);
      }
    }
    
  }

}
