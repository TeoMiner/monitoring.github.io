#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//--------------------khởi tạo giá trị chân DHT22-----------------
#define DHT_SENSOR_PIN  D7 
#define DHT_SENSOR_TYPE DHT22
//----------------------------------------------------------------

#define R  D8
#define G  D6
#define B  D5

const char* ssid = "dtp";
const char* password = "123456788";
const char* mqtt_server = "broker.hivemq.com";

const long utcOffsetInSeconds = 25200;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int t=0;


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
String message = "";
DHT dhtA(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
LiquidCrystal_I2C lcd(0x27,20,4); 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.windows.com", utcOffsetInSeconds);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int i=10;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(i,0);
    lcd.print(".");
    ++i;
    if(i==20) 
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Connecting");
      i=10;
    }
  }

  randomSeed(micros());

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connected to Wifi");
  delay(500);
}

void callback(char* topic, byte* payload, unsigned int length) {
  message = "";
  for (unsigned int i=0; i< length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Connecting to server");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      client.publish("reset","ok");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Connected to server!");
      client.subscribe("giatringuong");
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  delay(500);
}
void welcome()
{
  lcd.setCursor(0,0);
  lcd.print("----Temperature-----");
  lcd.setCursor(0,1);
  lcd.print("------Humidity------");
  lcd.setCursor(0,2);
  lcd.print("-----Monitoring-----");
  lcd.setCursor(0,3);
  lcd.print("-------System-------");
  delay(500);

}
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  welcome();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dhtA.begin();
}

void loop() 
{
  if (!client.connected()) {
    reconnect();
  }
  timeClient.update();
  client.loop();
  float temp = dhtA.readTemperature(); 
  float humid = dhtA.readHumidity(); 
  sprintf(msg, "Temperature: %f, Humidity: %f \n",temp ,humid);
  //client.publish("subfromesp", msg);
  client.publish("Nhietdo", String(temp).c_str());
  client.publish("Doam", String(humid).c_str());
  lcd.setCursor(0,0);
  lcd.print("Temperature: ");
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(humid);
  lcd.print("%");
  lcd.setCursor(0,2);
  lcd.print("Door: ");
  int s = analogRead(A0);
  if(s<70)
  {
    lcd.print("Closed");
    client.publish("Cua", "Closed");
    digitalWrite(R, LOW);
    digitalWrite(G, HIGH);
    digitalWrite(B, LOW);
  }    
  else 
  {
    lcd.print("Opened");
    client.publish("Cua", "Opened");
    digitalWrite(R, HIGH);
    digitalWrite(G, LOW);
    digitalWrite(B, LOW);
  }
  while(t<3)
  {
    lcd.setCursor(0,3);
    lcd.print(daysOfTheWeek[timeClient.getDay()]);
    lcd.print(",");
    lcd.print(timeClient.getFormattedTime());
    delay(500);
    if(temp<message.toInt())
    {
    analogWrite(R,255);
    analogWrite(G,255);
    analogWrite(B,0);
    }
    t++;
    if(t==3)
    {
      t=0;
      break;
    }
  }
}
