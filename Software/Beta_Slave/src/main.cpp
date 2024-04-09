#include <patient_monitor.h>

// // put function declarations here:

// void setup() {
//   // put your setup code here, to run once:
//   Wire.begin(4);
//   Wire.onReceive(receiveEvent);
//   Serial.begin(115200);
//   Serial.println("Hello from startup");

// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   //receiveData();
// }

// void receiveData()
// {
//   //connect to slave ESP
//   Wire.begin(slaveSDA, slaveSCL);
//   Wire.onReceive(receiveEvent);

//   //get time
//   time_t t = time(NULL);
//   struct tm tm = *localtime(&t);
//   sprintf(Patientdata.date, "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, 
//   tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

//   //send data via I2C to other esp (print values for now)
//   Serial.print("Spo2: ");
//   Serial.println(Patientdata.Spo2);
//   Serial.print("Spo2 Valid? ");
//   Serial.println(Patientdata.SpO2_invalid);
//   Serial.print("BP: ");
//   Serial.println(Patientdata.BP);
//   Serial.print("BP Valid? ");
//   Serial.println(Patientdata.BP_invalid);
//   Serial.print("HR: ");
//   Serial.println(Patientdata.Heartrate);
//   Serial.print("Date: ");
//   Serial.println(Patientdata.date);
//   Serial.print("ECG: ");

//   Wire.beginTransmission(127);
//   Wire.write("Patient Data Incoming");
//   Wire.write(Patientdata.Spo2);
//   Wire.write(Patientdata.SpO2_invalid);
//   Wire.write(Patientdata.BP);
//   Wire.write(Patientdata.BP_invalid);
//   Wire.write(Patientdata.Heartrate);
//   Wire.write(Patientdata.date);
//   Wire.endTransmission(true);

//   //reset all values back to 0
//   Patientdata.Spo2 = 0;
//   Patientdata.SpO2_invalid = 0;
//   Patientdata.BP_invalid = 0;
//   Patientdata.ECG_invalid = 0;
//   Patientdata.Heartrate = 0;
//   strcpy(Patientdata.BP, " ");
//   strcpy(Patientdata.date, " ");

//   for(int i = 0; i < 1000; i++)
//   {
//     Serial.print(Patientdata.ECG[i]);
//     Patientdata.ECG[i] = 0;
//   }
// }

// void receiveEvent(int howMany)
// {
//   while(1 < Wire.available()) // loop through all but the last
//   {
//     char c = Wire.read(); // receive byte as a character
//     Serial.print(c);         // print the character
//   }
//   int x = Wire.read();    // receive byte as an integer
//   Serial.println(x);         // print the integer
// }

int i = 0;
int temp_flag = 0;
volatile int master_selection;
patientdata Patientdata; // Global variable to store received data

void setup() {
  Serial.println("In setup");
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Serial.begin(115200);
}

void loop() {
  // Slave does nothing in loop
  
  if(i = 0)
  {
    Serial.println("In slave loop");
    Wire.begin(127);
    Wire.onReceive(receiveEvent);
    i++;
  }
  //Serial.println("In slave loop");
}

/*
void receiveEvent(int howMany) {
  if (howMany >= sizeof(patientdata)) {
    uint8_t buffer[sizeof(patientdata)];
    for (int i = 0; i < sizeof(patientdata); i++) {
      buffer[i] = Wire.read();
    }

    // Deserialize the received data
    deserializePatientData(Patientdata, buffer);

    // Print received data (optional for debugging)
    Serial.print("Received Spo2: ");
    Serial.println(Patientdata.Spo2);
    Serial.print("Received BP: ");
    Serial.println(Patientdata.BP);
    Serial.print("Received HR: ");
    Serial.println(Patientdata.Heartrate);
    Serial.print("Received Date: ");
    Serial.println(Patientdata.date);
    Serial.print