/*******************************************************************************
 * Programmers: Jake Lewis, Zachary Harrington, Nicholas Gerth, Matthew Stavig *                                                      
 * Class: EE415 - Product Design Management                                    *
 * Sponsoring Company: Philips                                                 *
 * Industry Mentor: Scott Schweizer                                            *
 * Faculty Mentor: Mohammad Torabi Konjin                                      *
 *                                                                             *
 *                          Patient Monitor Project                            *
 *                                                                             *
 * Date: 4/15/2024                                                             *
 * File: main.cpp                                                              *
 *                                                                             *
 * Description: A patient monitor measuring the three most important           *
 *              physilogical parameters: blood oxygen, ECG, and blood pressure *   
 *                                                                             *
 *                                                                             *
 ******************************************************************************/

#include <patient_monitor.h>

// Variables to store received data from ESP32
String patientID = "";
String patientName = "";
String patientAge = "";
String patientSex = "";
String timestamp = "";
String bloodPressure = "";
String heartRate = "";
String spO2 = "";

// Flag to indicate if ESP32 has sent data
bool dataReceived = false;

int i = 0;
int temp_flag = 0;
volatile int master_selection;
patientdata Patientdata; // Global variable to store received data

// Replace with your network credentials
const char* ssid = "LAPTOP-OSBR9R3C 2190";
const char* password = "*69048uB";

// Set web server port number to 80
WiFiServer server(80);

// ECG variables
double ECG_QRS_signal[ECG_CORRELATION_LENGTH] = {2.19, 2.21, 2.22, 2.26, 2.29, 2.31, 2.33, 2.37, 2.38, 2.37, 
                                            2.31, 2.25, 2.17, 2.08, 2.03, 1.99, 1.97, 1.96, 1.94, 1.94};
uint16_t ECG_correlation_data[ECG_CORRELATION_LENGTH] = {0};
uint16_t ECG_beat_threshold = 4750;
bool ECG_above_beat_threshold = false;
int64_t ECG_beat_times[ECG_HEARTRATE_SAMPLES] = {0};

// Define the SPI bus configuration
spi_bus_config_t bus_config = {
    .miso_io_num = 14,  // MISO connected to GPIO14 (IO14)
    //.mosi_io_num = -1,  // Not used for ADC121
    .sclk_io_num = 13,  // SCLK connected to GPIO13 (IO13)
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 0,
};

// Define the SPI device configuration
spi_device_interface_config_t dev_config = {
    .mode = 0,                  // SPI mode 0
    .clock_speed_hz = 1000000,  // 1MHz min for ADC121
    .spics_io_num = 12,         // CS connected to GPIO12 (IO12)
    .queue_size = 1,
};

spi_device_handle_t spi;

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

  // Initialize ECG SPI interface
  init_ECG();
}

void loop() {
  // Slave does nothing in loop
  
  // if(i = 0)
  // {
  //   Serial.println("In slave loop");
  //   Wire.begin(127);
  //   Wire.onReceive(receiveEvent);
  //   i++;
  // }
  
  // connect to web server
  connectserver();
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

void connectserver(){
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
        // Update HTML content with received variables
        html = String(HTML_CONTENT_SUBJECT1);
        html.replace("$patientID", patientID);
        html.replace("$patientName", patientName);
        html.replace("$patientAge", patientAge);
        html.replace("$patientSex", patientSex);
        html.replace("$timestamp", timestamp);
        html.replace("$bloodPressure", bloodPressure);
        html.replace("$heartRate", heartRate);
        html.replace("$spO2", spO2);
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

  // Variables to send data from ESP32
 patientName = Patientdata.name;
 patientAge = random(100);
 patientSex = "Male";

timestamp = Patientdata.date;
bloodPressure = Patientdata.BP;
heartRate = Patientdata.Heartrate;
spO2 = Patientdata.Spo2;

  for(int i = 0; i < 1000; i++)
  {
    Patientdata.ECG[i] = i;
    Serial.println(Patientdata.ECG[i]);
  }
  Serial.print("ECG valid: ");
  Serial.println(Patientdata.ECG_invalid);


  //send data to webserver here
  dataReceived = true;
  handleVariables();

}

void handleVariables() {
  // Receive variables from ESP32
  if (dataReceived) {
    generate_id_number();
    // Update HTML content with received variables
    String html = String(HTML_CONTENT_SUBJECT1);
    html.replace("$patientID", patientID);
    html.replace("$patientName", patientName);
    html.replace("$patientAge", patientAge);
    html.replace("$patientSex", patientSex);
    html.replace("$timestamp", timestamp);
    html.replace("$bloodPressure", bloodPressure);
    html.replace("$heartRate", heartRate);
    html.replace("$spO2", spO2);
    
    // Print updated HTML to serial monitor
    //Serial.println(html);
    
    dataReceived = false; // Reset flag
  }
}

void generate_id_number(){
  int random_id = random(10000); // Generate a random number between 0 and 9999

  patientID = String(random_id);

  if (random_id < 1000) {
    patientID = "0" + patientID;  
  }
  if (random_id < 100) {
    patientID = "0" + patientID;
  }
   if (random_id < 10) {
    patientID = "0" + patientID;
  }
}

void init_ECG()
{
  esp_err_t ret;

  // Initialize the SPI bus
  ret = spi_bus_initialize(VSPI_HOST, &bus_config, 1);
  assert(ret == ESP_OK);

  // Add the SPI device
  
  ret = spi_bus_add_device(VSPI_HOST, &dev_config, &spi);
  assert(ret == ESP_OK);
}

void ECG_Measurement()
{
  Serial.println("In ECG Measurement");

  for (int index = 0; index < ECG_SAMPLES; index++)
  {
    // Read data from ADC121
    uint8_t adc_data[2];
    spi_transaction_t trans = {
        .length = 16, // 12-bit ADC, so 16 bits
        .tx_buffer = NULL,
        .rx_buffer = adc_data,
    };
    esp_err_t ret = spi_device_transmit(spi, &trans);
    assert(ret == ESP_OK);

    // Convert data to integer
    uint16_t raw_value = (adc_data[0] << 8) | adc_data[1];

    // Add data to arrays
    Patientdata.ECG[index] = raw_value;
    for (int i = 0; i < ECG_CORRELATION_LENGTH-1; i++) {
        ECG_correlation_data[i] = ECG_correlation_data[i+1];
    }
    ECG_correlation_data[ECG_CORRELATION_LENGTH-1] = raw_value;
    
    // *** Add code to update webserver/app ***
    //Serial.println(raw_value);

    // Cross Correlation
    double sum = 0;
    for (int i = 0; i < ECG_CORRELATION_LENGTH; i++) {
        sum += ECG_correlation_data[i] * ECG_QRS_signal[i];
    }
    uint16_t coorelated_value = (uint16_t)(sum/ECG_CORRELATION_LENGTH);
    
    // Calculate Heartrate
    if (ECG_above_beat_threshold && coorelated_value < ECG_beat_threshold) {
        ECG_above_beat_threshold = false;
    } else if (!ECG_above_beat_threshold && coorelated_value > ECG_beat_threshold) {
        ECG_above_beat_threshold = true;

        for (int i = 0; i < ECG_HEARTRATE_SAMPLES-1; i++) {
            ECG_beat_times[i] = ECG_beat_times[i+1];
        }
        ECG_beat_times[ECG_HEARTRATE_SAMPLES-1] = esp_timer_get_time();

        double heartrate = ((ECG_HEARTRATE_SAMPLES-1) / ((ECG_beat_times[ECG_HEARTRATE_SAMPLES-1] - ECG_beat_times[0]) / 1000000.0)) * 60;
        Patientdata.Heartrate = heartrate;
        Serial.println(Patientdata.Heartrate);
    }
    delay(10);
  }

  // byte TxByte = 0;

  // if(temp_flag == 0)
  // {
  // //temp for testing
  //  for(int i = 0; i < 255; i++)
  //  {
  //   //Wire.beginTransmission(SLAVE_ADDRESS); // transmit to device 127
  //   Patientdata.ECG[i] = i;
  //   //Wire.write((byte)Patientdata.ECG[i]);
  //   Wire.write("a");
  //   Serial.print("Data: ");
  //   Serial.println(Patientdata.ECG[i]);
  //   //Wire.endTransmission();    // stop transmitting
  //  }  

  //  temp_flag = 1;
  // }
   
}

