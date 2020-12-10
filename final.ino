#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
String readString;
//WiFi SSID
const char* ssid = "srinuv";
//WiFi Password
const char* password = "12356789";
//Host Name for Google Sheets
const char* host = "script.google.com";
//Host Name for Thing Speak
const char* server = "api.thingspeak.com";
const int API_TIMEOUT = 10000;
//Host Name for ifttt  
const char* host1 = "maker.ifttt.com";
//HttpsPort For Communication
const int httpsPort = 443;
//Create a Object for Secure Connection to URL's
WiFiClientSecure client;
//WiFiClient client1;
//FingerPrint of Googel Sheet Website
const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
 // Google Sheet service ID
String GAS_ID = "AKfycby69de53RKjPhqS75ha1XBJBw6soWdxsriKHtK6_JdFQb1nigo";
//Things Speak API ID
String apiKey = "Y5874TKIR8FXV2XS";
//Node D6,D5 pins are used for serial communication
SoftwareSerial s(D6,D5);
void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  //Software serial for recieving from arduino
  s.begin(9600);
  while (!Serial) continue;
  //D1 pin is used for buzzer
  pinMode(D1,OUTPUT);
  //STA stands for Station mode it is mode which is used to connect to a WiFi Access Point
  WiFi.mode(WIFI_STA);
  //Connecting to WiFi with given SSID and PassWord
  WiFi.begin(ssid, password);
  //Waiting till the WiFi is connected to the NodeMCU
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  //Sending Test Data to Google Sheets       
   sendData(113,125,0);
   pinMode(D2,OUTPUT);  
}
 
void loop() {
  /*Serial Transfer Code*/
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(s);
  if (root == JsonObject::invalid())
    return;
  Serial.println("Data Recieved From Arduino");
//  root.prettyPrintTo(Serial);
  Serial.print("gas= ");
  int gas=root["gas"];
  Serial.println(gas);
  Serial.print("temp= ");
  int temp=root["temp"];
  Serial.println(temp);
  int light=root["light"];
   Serial.print("light =");
  Serial.println(light);
  if(temp>30){
    digitalWrite(D2,LOW);
  }
  else
  digitalWrite(D2,HIGH);
  if(temp>30||light>1000)  /*Turns On Buzzer */
  digitalWrite(D1,HIGH);
  else
  digitalWrite(D1,LOW);
  if(gas>500){
    digitalWrite(D1,HIGH);
    sendiftt();
  }

  /* Serial Transfer Code Ends */
  /*Code For google Sheet Entry*/
  sendThing(gas,temp);
  sendData(gas,temp,light);
  
}
//Function for seding the code to google sheet
void sendData(int x, int y,int z)
{
  //Establishing a Secure Connection
  client.setInsecure();
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  if (client.verify(fingerprint, host)) {
        Serial.println("certificate matches");
   } 
  else {
      Serial.println("certificate doesn't match");
   }
  //Convert the values we got from Arduino into Strings to send to Google sheet
  String string_x=String(x, DEC);
  String string_y=String(y, DEC);
  String string_z=String(z,DEC);
  //The below is the URL Which need to be called to store the Data in the Google Sheet
  String url = "/macros/s/" + GAS_ID + "/exec?value1=" + string_x + "&value2=" + string_y+"&value3="+string_z;
  Serial.print("requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +"Host: " + host + "\r\n" +"User-Agent: BuildFailureDetectorESP8266\r\n" +"Connection: close\r\n\r\n");
  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
//      Serial.println("headers received");
      break;
    }
  }
  client.stop();
}
void sendThing(int x,int y){
  //Connecting to ThingsSpeak Server Througth the PORT 80;
  client.setInsecure();
  Serial.print("connecting to ");
  Serial.println(server);
  
    if (client.connect(server,443))
    {
          String postStr = apiKey+"&field1="+String(x)+"&field2="+String(y)+"\r\n\r\n";
          client.print("POST /update HTTP/1.1\n");
          client.print("Host: api.thingspeak.com\n");
          client.print("Connection: close\n");
          client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
          client.print("Content-Type: application/x-www-form-urlencoded\n");
          client.print("Content-Length: ");
          client.print(postStr.length());
          client.print("\n\n");
          client.print(postStr);
          Serial.println("Data Sent to Things speak");
     }
       client.stop();
       Serial.println("Waiting....");
       delay(9000);
}
void sendiftt(){
  
//  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(API_TIMEOUT);
  Serial.print("connecting to ");
  Serial.println(host1);
  if (!client.connect(host1, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/trigger/Mini1/with/key/i1iOXOLTnys1f5mbkSR0b83EJKEUv38YbEZzUwcRONI";
  Serial.print("requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host1 + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
//      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
}
