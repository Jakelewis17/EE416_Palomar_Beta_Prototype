#include <patient_monitor.h>


int i = 0;
int temp_flag = 0;
volatile int master_selection;
patientdata Patientdata; // Global variable to store received data

// Replace with your network credentials
const char* ssid = "LAPTOP-OSBR9R3C 2190";
const char* password = "*69048uB";

// Set web server port number to 80
WiFiServer server(80);

void setup() {
  Serial.println("In setup");
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    // wait for connection
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
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
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    
    //Serial.println("New Client.");          // print a message out in the serial port
    //String currentLine = "";                
    // read first line of HTTP request header
    String HTTP_request = "";
    
    while (client.connected()) {
      if (client.available()) {
        Serial.println("New HTTP Request");
        HTTP_request = client.readStringUntil('\n'); // read first line of HTTP request
        Serial.print("<< ");
        Serial.println(HTTP_request); // print HTTP request to the serial monitor
        break;
      }
    }

    // read reamaing lines of HTTP request header
    while (client.connected()) {
      if (client.available()) {             // if there's bytes to read from the client,
      String Header = client.readStringUntil('\n'); // read the header line of HTTP request

      if (Header.equals("\r")) // end HTTP request
      break;

      Serial.print("<< ");
      Serial.println(Header); // print HTTP request to monitor
      }
    }

    int page_id = 0;

    if (HTTP_request.indexOf("GET") == 0) {
      if (HTTP_request.indexOf("GET / ") > -1 || HTTP_request.indexOf("GET /index ") > -1 || HTTP_request.indexOf("GET /index.html ") > -1){
        Serial.println("Home Page!");
        page_id = INDEX_PAGE;
      }
      else if (HTTP_request.indexOf("GET /subject1 ") > -1 || HTTP_request.indexOf("GET /subject1.html ") > -1){
        Serial.println("Subject1 Page!");
        page_id = SUBJECT1_PAGE;
      }
      else { //Page not found
        Serial.println("Page Not Found (Error 404)");
        page_id = ERROR_404_PAGE;
      }
    }
    else { // Method Not Allowed
      Serial.println("Method Not Allowed (Error 405)");
      page_id = ERROR_405_PAGE;
    }
    
    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
    // and a content-type so the client knows what's coming, then a blank line:
    if (page_id == ERROR_404_PAGE)
      client.println("HTTP/1.1 404 Error Page Not Found");
    if(page_id == ERROR_405_PAGE)
      client.println("HTTP/1.1 Error 405 Method Not Allowed");
    else
      // connected to webpage
      client.println("HTTP/1.1 200 OK");
    
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();    

    String html;
    switch(page_id){
      case INDEX_PAGE:
        //client.println("This is the home page");
        html = String(HTML_CONTENT_HOME);
        break;
      case SUBJECT1_PAGE:
        //client.println("This is the Subject1 page");
        html = String(HTML_CONTENT_SUBJECT1);
        break;
      case ERROR_404_PAGE:
        //client.println("Page Not Found");
        html = String(HTML_CONTENT_404);
        break;
      case ERROR_405_PAGE:
        //client.println("Method Not Allowed");
        html = String(HTML_CONTENT_405);
        break;
    }

    client.println(html);
    client.flush();

    // give the web browser time to receive the data
    delay(10); 

    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void deserializePatientData(patientdata& data, const uint8_t* buffer) {
    memcpy(&data, buffer, sizeof(patientdata));
}

void receiveEvent(int howMany)
{
  Serial.println("In receive event");
  master_selection = Wire.read();    // receive byte as an integer
  Serial.println((int)master_selection);

  if(master_selection == 0) //receive patientdata and send to webserver
  {
    send_to_webserver();
  }
  else if (master_selection == 1) //Do ECG measurement and send data back 
  {
    ECG_Measurement();
  }
  
}



void send_to_webserver()
{
  
  int SpO2 = Wire.read();    // receive byte as an integer
  Patientdata.Spo2 = SpO2;   //populate Patientdata data structure

  int SpO2_valid = Wire.read();
  Patientdata.SpO2_invalid = SpO2_valid;

  for(int i = 0; i < 6; i++)
  {
    Patientdata.BP[i] = Wire.read();
  }
  Patientdata.BP[7] = '\0';
  int BP_valid = Wire.read();
  Patientdata.BP_invalid = BP_valid;

  byte heartrate = Wire.read();
  Patientdata.Heartrate = (int)heartrate;

  for(int i = 0; i < 7; i++)
  {
    Patientdata.date[i] = Wire.read();
  }

  //get any extra data
  while(Wire.available())
  {
    byte garbage = Wire.read();
  }

  //print for check
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
  for(int i = 0; i < 1000; i++)
  {
    Patientdata.ECG[i] = i;
    Serial.println(Patientdata.ECG[i]);
  }
  Serial.print("ECG valid: ");
  Serial.println(Patientdata.ECG_invalid);


  //send data to webserver here


}


void ECG_Measurement()
{
  Serial.println("In ECG Measurement");
   //Zack ECG code here

   byte TxByte = 0;

  if(temp_flag == 0)
  {
  //temp for testing
   for(int i = 0; i < 255; i++)
   {
    //Wire.beginTransmission(SLAVE_ADDRESS); // transmit to device 127
    Patientdata.ECG[i] = i;
    //Wire.write((byte)Patientdata.ECG[i]);
    Wire.write("a");
    Serial.print("Data: ");
    Serial.println(Patientdata.ECG[i]);
    //Wire.endTransmission();    // stop transmitting
   }  

   temp_flag = 1;
  }
   
}

