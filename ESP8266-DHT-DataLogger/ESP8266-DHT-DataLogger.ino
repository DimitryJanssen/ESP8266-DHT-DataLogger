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

// Update these with values suitable for your network.
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Update these with values suitable for your MQTT config.
const char* mqtt_server = "172.16.10.10"; //IP or DNS Hostname
const char* mqtt_clientid = "NodeMCU01";
const char* mqtt_username = "USERNAME";
const char* mqtt_password = "PASSWORD";
const char* mqtt_topic = "tele/NodeMCU/SENSOR";

//NTP Configuration:
//const char* ntp_host = "172.16.10.10" //IP or DNS Hostname
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
  #define LED D4 
   pinMode(LED, OUTPUT); 
   noInterrupts(); 
   timer0_isr_init(); 
   timer0_attachInterrupt(timer0_ISR); 
   //timer0_write(ESP.getCycleCount() + 80000000L); // 80MHz == 1sec 
   interrupts();
}

void timer0_ISR (void) 
{
  if (digitalRead(LED) == HIGH)
  {
    digitalWrite(LED, LOW);
    timer0_write(ESP.getCycleCount() + 5000000L); // 80MHz == 1sec
  }
  else
  {
    digitalWrite(LED, HIGH);
    timer0_write(ESP.getCycleCount() + 240000000L); // 80MHz == 1sec
  }
} 
  
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
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
    client.publish(mqtt_topic, message);
  }
}
