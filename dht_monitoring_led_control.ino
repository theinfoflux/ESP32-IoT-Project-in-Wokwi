#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6uY4gJNfn"
#define BLYNK_TEMPLATE_NAME "DHT Monitoring"
#define BLYNK_AUTH_TOKEN "bEU5TUGhXdHQZ9zYC8clDzMXc95n2gt3"



#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>


#include <DHT.h>

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "Wokwi-GUEST"; //nama hotspot yang digunakan
char pass[] = ""; //password hotspot yang digunakan

#define DHTPIN 12          // Mention the digital pin where you connected 
#define DHTTYPE DHT22     // DHT 11  
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
int ledpin = 14;


void setup(){
   Serial.begin(115200);
   pinMode(ledpin,OUTPUT);
  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
  
  dht.begin();
  timer.setInterval(2500L, sendSensor);
}

void loop(){
  Blynk.run();
  timer.run();
}
void sendSensor(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V0, t);
  Serial.print("Temperature : ");
  Serial.print(t);
  Serial.print("    Humidity : ");
  Serial.println(h);


  if(t > 30){
   // Blynk.email("isamahfudi08@gmail.com", "Alert", "Temperature over 28C!");
    Blynk.logEvent("temp_alert","Temp above 30 degrees");
  }
}
 BLYNK_WRITE(V2) // From Blynk to the ESP32, to control a Relay
{
  int pinValue1 = param.asInt();
  digitalWrite(ledpin , pinValue1);
  }