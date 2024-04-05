#include <slave.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

void receiveData()
{
  //connect to slave ESP
  Wire.begin(slaveSDA, slaveSCL);

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

  Wire.beginTransmission(127);
  Wire.write("Patient Data Incoming");
  Wire.write(Patientdata.Spo2);
  Wire.write(Patientdata.SpO2_invalid);
  Wire.write(Patientdata.BP);
  Wire.write(Patientdata.BP_invalid);
  Wire.write(Patientdata.Heartrate);
  Wire.write(Patientdata.date);
  Wire.endTransmission(true);

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