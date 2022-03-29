#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>

#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_SSD1306.h>  // include Adafruit SSD1306 OLED display driver
Adafruit_SSD1306 display(128,64,&Wire,-1);

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID        "XXXXXXXXXXXXXXX"
#define WLAN_PASS        "XXXXXXXXXXXXXXXX"


/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "XXXXXXXXXXXXXXXXX"
#define AIO_KEY         "XXXXXXXXXXXXXXXXX" 

WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/ 
Adafruit_MQTT_Publish Temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish Hu = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/hu");
Adafruit_MQTT_Publish SensorValue= Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/sensorValue");
//Adafruit_MQTT_Publish latitude = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/latitude");
//Adafruit_MQTT_Publish longitude = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/longitude");

// lora Module Pins
#define SS 5
#define RST 14
#define DI0 2
 
//#define TX_P 17
#define BAND 433E6
//#define ENCRYPT 0x78
 
String device_id;
String temp;
String hu;
String sensorValue;
 
WebServer server(80);
 
void setup()
{
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
 
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  display.println("LoRa Receiver");
  display.display();

  Serial.println("LoRa Receiver");
 
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND))
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  Serial.println("Connecting to ");
  Serial.println(WLAN_SSID);
 
  //Connect to your local wi-fi network
  WiFi.begin(WLAN_SSID, WLAN_PASS);
 
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("IP address: ");  
  Serial.println(WiFi.localIP());
  
  connect();
}

// connect to adafruit io via MQTT
void connect() {
  Serial.print(F("Connecting to Adafruit IO... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(10000);
  }
  Serial.println(F("Adafruit IO Connected!"));
}


 
void loop()
{

  // ping adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }


  // try to parse packet
  int pos1, pos2, pos3; //, pos4, pos5;
 
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    // received a packet
    Serial.print("Received packet:  ");
    String LoRaData = LoRa.readString();
    Serial.print(LoRaData);
    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
 
    pos1 = LoRaData.indexOf('/');
    pos2 = LoRaData.indexOf('&');
    pos3 = LoRaData.indexOf('#');
 
    device_id = LoRaData.substring(0, pos1);
    temp = LoRaData.substring(pos1 + 1, pos2);
    hu = LoRaData.substring(pos2 + 1, pos3);
    sensorValue = LoRaData.substring(pos3 + 1, LoRaData.length());
 
    Serial.print(F("Device ID = "));
    Serial.println(device_id);
 
    Serial.print(F("Temperature = "));
    Serial.print(temp);
    Serial.println(F("*C"));
 
    Serial.print(F("Humidity = "));
    Serial.print(hu);
    Serial.println(F("hPa"));
 
    Serial.print(F("Air Quality = "));
    Serial.print(sensorValue);
    Serial.println(F("m"));

//    Serial.print("Latitude  = ");
//    Serial.print(latitude);
//    Serial.println("%");
//  
//    Serial.print(F("Longitude = "));
//    Serial.print(longitude);
//    Serial.println(F("%"));


    if (! Temp.publish(temp.toFloat())) {                                        //Publish to Adafruit
      Serial.println(F("Failed"));
    } 
    if (! Hu.publish(hu.toFloat())) {                                            //Publish to Adafruit
      Serial.println(F("Failed"));
    }
    if (! SensorValue.publish(sensorValue.toFloat())) {                          //Publish to Adafruit
      Serial.println(F("Failed"));
    }
//    if (! SoilMoisture.publish(latitude.toFloat())) {                          //Publish to Adafruit
//      Serial.println(F("Failed"));
//    }
//    if (! Rainfall.publish(longitude.toFloat())) {                             //Publish to Adafruit
//      Serial.println(F("Failed"));
//    }
//   else {
//      Serial.println(F("Sent!"));
//    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20, 0);
    display.println("LoRa Receiver");
  
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.print("RSSI: ");
    display.println(LoRa.packetRssi());
    display.print("Temperature: ");
    display.println(temp);
    display.print("Humidity: ");
    display.println(hu);
    display.print("Airquality: ");
    display.println(sensorValue);
    display.display();
    Serial.println();

  }
}
