#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>


// DHT Sensor configuration:
// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

const int DHTPin = D3; //sensor connected to pin:

// Initialize DHT sensor.
DHT dht(DHTPin,DHTTYPE);

// Temporary variables
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];

// Update these with values suitable for your network.
const char* ssid = "6852JR54";
const char* password = "19??W!ldzang";

// Update these with values suitable for your MQTT config.
const char* mqtt_server = "172.16.10.10";
const char* mqtt_clientid = "NodeMCU";
const char* mqtt_username = "mqtt-sonoff";
const char* mqtt_password = "19??Mqtt";

//NTP Configuration:
//const char* ntp_host = "172.16.10.10"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
   delay(100);
  // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
  randomSeed(micros());
  timeClient.update();
  Serial.println("");
  Serial.println(timeClient.getFormattedDate());
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Command is : [");
  Serial.print(topic);
  int p =(char)payload[0]-'0';
  // int chk = dht(DHTPin);
  // if MQTT comes a 0 message, show humidity
  if(p==0) 
  {
    Serial.println("to show humidity!]");
    Serial.print(" Humidity is: " );
    Serial.print(dht.readHumidity());
    Serial.println('%');
  } 
  // if MQTT comes a 1 message, show temperature
  if(p==1)
  {
  // digitalWrite(BUILTIN_LED, HIGH);
   Serial.println(" is to show temperature!] ");
   //int chk = dht(DHTPin);
   Serial.print(" Temp is: " );
   Serial.print(dht.readTemperature());
   Serial.println(' C');
  }
  Serial.println();
} //end callback

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    //if (client.connect(clientId.c_str()))
    if (client.connect(mqtt_clientid,mqtt_username,mqtt_password))
    {
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command if any
      client.subscribe("NodeMCU");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 6 seconds before retrying
      delay(6000);
    }
  }
} //end reconnect()

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //int chk = dht(DHTPin);
  Serial.print(" Starting Humidity: " );
  Serial.print(dht.readHumidity());
  Serial.println('%');
  Serial.print(" Starting Temparature ");
  Serial.print(dht.readTemperature());
  Serial.println('C');
  //LED Configuration:
  pinMode(D4, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  digitalWrite(D4, LOW);           // Turn the LED on (Note that LOW is the voltage level
  delay(125);
  digitalWrite(D4, HIGH);          // Turn the LED off by making the voltage HIGH
  delay(875);
  
  client.loop();
  long now = millis();
  // read DHT11 sensor every 60 seconds
  if (now - lastMsg > 60000) {
     lastMsg = now;
     //int chk = dht(DHTPin);
     String
     msg="{\"Time\":\"";
     msg=msg+timeClient.getFormattedDate();
     msg=msg+"\",\"AM2301\":{\"Temperature\":";
     msg=msg+dht.readTemperature();
     msg=msg+", \"Humidity\":" ;
     msg=msg+dht.readHumidity();
     msg=msg+"}}";
     char message[90];
     msg.toCharArray(message,90);
     Serial.println(message);
     //publish sensor data to MQTT broker
    client.publish("tele/NodeMCU/SENSOR", message);
  }
}
