#define IN A1
#define LENGTH 9//21

//double knownSignal[LENGTH] = {0, 0.20, 0.20, 0.20, 0.25, 0.35, 0.45, 0.55, 0.58, 0.56, 0.25, 0.35, 0.38, 0.35, 0.20, 0.20, 0.20, 0.25, 0.05, 1.40, 2.55, };
double knownSignal[LENGTH] = {0.25, 0.25, 0.1, 1.40, 2.55, 1.40, 0, 0.25, 0.25};
int unknownSignal[LENGTH] = {0};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // Read analog input and store in the unknown signal buffer
  int data = analogRead(IN);
  for (int i = 0; i < LENGTH-1; i++) {
    unknownSignal[i] = unknownSignal[i+1];
  }
  unknownSignal[LENGTH-1] = data;

  // 500Hz -> 2ms
  delay(2); // Adjust delay according to sampling rate
  
  double sum = 0;
  for (int i = 0; i < LENGTH; i++) {
    sum += knownSignal[i] * unknownSignal[i];
  }

  Serial.print(1500);
  Serial.print(", ");
  Serial.print(0);
  Serial.print(", ");
  Serial.print(data);
  Serial.print(", ");
  Serial.println(sum*3/LENGTH);


}

