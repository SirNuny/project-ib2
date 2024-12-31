/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include <SoftwareSerial.h>

SoftwareSerial mySerial(13,15);

// Update these with values suitable for your network.

const char* ssid = "zak";
const char* password = "wattafuckareyouguysdoing";
const char* mqtt_server = "192.168.137.1";

String dataString = "";
String subdataString = "";
String orgdataString = "";
int CO2 = 0, VOC = 0, temp = 0, humid = 0, light = 0;
DynamicJsonDocument doc(1024);
char json_data_out[1024];

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void get_uart_data(){
  // Condition to read the data from the UART bus
  while(mySerial.available()>0){
    dataString += char(mySerial.read());}

  // Datastring should be in the following format (31 characters)
  // "T:22.46,RH:36.84,CO2:400,VOC:62"; 
  
  // Condition to process the received UART data
  if(dataString.length() >= 31){

    // Store the orginal string
    orgdataString = dataString;
      
    // Write the raw UART data  
    Serial.print("Raw UART data:");
    Serial.println(dataString);
    // Process the raw UART data
    for (int ctr=0; ctr<5; ctr++){
  
      int startIdx = dataString.indexOf(':');
      int stopIdx = dataString.indexOf(',');
        
      subdataString = dataString.substring(startIdx+1, stopIdx);
      dataString = dataString.substring(stopIdx+1);         

      String tmpString = "";
      
      // Store in the correct variable
      if (ctr == 0){
        CO2 = subdataString.toInt();
        tmpString = "0: CO2 = " + String(CO2) + "˚C\r\n";
        }
      else if (ctr == 1){
        VOC = subdataString.toInt();
        tmpString = "1: VOC = " + String(VOC) + "%\r\n";
        }
      else if (ctr == 2){
        temp = subdataString.toInt();
        tmpString = "2: Temperature = " + String(temp) + "ppm\r\n";
        }
      else if (ctr == 3){
        humid = subdataString.toInt();
        tmpString = "3: Humidity = " + String(humid) + "ppb\r\n";
        }
      else if (ctr == 4){
        light = subdataString.toInt();
        tmpString = "4: Light = " + String(light) + "lux\r\n";
        }
    }
      dataString = "";
  }
}

void get_fixed_data(){
    String dataString = "T:22.46,RH:36.84,CO2:400,VOC:62"; 
 
    if(dataString.length() >= 31){

      // Store the orginal string
      orgdataString = dataString;
      
      // Write the raw UART data  
      Serial.print("Raw UART data:");
      Serial.println(dataString);
      // Process the raw UART data
      for (int ctr=0; ctr<5; ctr++){
  
        int startIdx = dataString.indexOf(':');
        int stopIdx = dataString.indexOf(',');
        
        subdataString = dataString.substring(startIdx+1, stopIdx);
        dataString = dataString.substring(stopIdx+1);         

        String tmpString = "";
      
        // Store in the correct variable
        if (ctr == 0){
          CO2 = subdataString.toInt();
          tmpString = "0: CO2 = " + String(CO2) + "˚C\r\n";
          }
        else if (ctr == 1){
          VOC = subdataString.toInt();
          tmpString = "1: VOC = " + String(VOC) + "%\r\n";
          }
        else if (ctr == 2){
          temp = subdataString.toInt();
          tmpString = "2: Temperature = " + String(temp) + "ppm\r\n";
          }
        else if (ctr == 3){
          humid = subdataString.toInt();
          tmpString = "3: Humidity = " + String(humid) + "ppb\r\n";
          }
        else if (ctr == 4){
          light = subdataString.toInt();
          tmpString = "4: Light = " + String(light) + "lux\r\n";
          }
      }
      dataString = "";
    }
}


void create_json(){
  doc["co2"] = CO2;
  doc["voc"] = VOC;
  doc["temperature"] = temp;
  doc["humidity"] = humid;
  doc["light"] = light;
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  mySerial.begin(115200);
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
 
  // Reconnect if required
  if (!client.connected()){
    reconnect();
  }
  client.loop();

  // Get UART data
  //get_fixed_data();
  get_uart_data();

  // Create JSON format
  create_json();
  // Publisch data
  serializeJson(doc, json_data_out);
  client.publish("sensor", json_data_out);
  Serial.println(json_data_out);
  
  // Wait for 1 second
  delay(1000);
     
}
