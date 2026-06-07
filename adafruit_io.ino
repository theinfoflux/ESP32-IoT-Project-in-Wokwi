#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"
//----------------------------------------

//----------------------------------------Variable declaration for your network credentials.

#define WLAN_SSID       "Wokwi-GUEST"
#define WLAN_PASS       ""
//----------------------------------------

//----------------------------------------
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    ""
#define AIO_KEY         ""
//----------------------------------------

// Defines PIN and DHT sensor type.
#define DHTPIN 26
#define DHTTYPE DHT22

// Defining Relays PIN.
#define Relay_Light_Bulb_1_PIN 12
#define Relay_Light_Bulb_2_PIN 14
#define Relay_Light_Bulb_3_PIN 27
// Defining on_Board_LED PIN.
#define on_Board_LED  2

float temperature_Val = 0.0;
float humidity_Val = 0;

// Declaring the "DHT" object as "dht11" and its settings.
DHT dht11(DHTPIN, DHTTYPE);

// Declaring the "WiFiClient" object as "client".
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Subscribe led1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led1");
Adafruit_MQTT_Subscribe led2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led2");
Adafruit_MQTT_Subscribe led3 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led3");


//________________________________________________________________________________ MQTT_Connect()
void MQTT_Connect() {
  int8_t ret;
  
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.println();
  Serial.println("-------------MQTT_Connect()");
  Serial.println("Connecting to MQTT...");
  
  // connect will return 0 for connected.
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 10 seconds...");
    mqtt.disconnect();
    delay(5000); // wait 5 seconds.
  }
  
  Serial.println("MQTT Connected.");
  Serial.println("-------------");
}
//________________________________________________________________________________ 



//________________________________________________________________________________ read_DHT11()
void read_DHT11() {
  Serial.println();
  Serial.println("-------------read_DHT11()");
  
  // Read humidity.
  humidity_Val = dht11.readHumidity();
  // Read temperature as Celsius (the default).
  temperature_Val = dht11.readTemperature();

  // Check if any reads failed.
  if (isnan(temperature_Val) || isnan(humidity_Val)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    Serial.println();
    humidity_Val = 0;
    temperature_Val = 0.00;
  }

  Serial.print(F("Humidity: "));
  Serial.print(humidity_Val);
  Serial.print(F("% ||  Temperature: "));
  Serial.print(temperature_Val);
  Serial.println(F("°C"));
  Serial.println("-------------");
}
//________________________________________________________________________________ 



//________________________________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:

  delay(2000);
  Serial.begin(115200);
  Serial.println();

  pinMode(on_Board_LED, OUTPUT);
  pinMode(Relay_Light_Bulb_1_PIN, OUTPUT);
  pinMode(Relay_Light_Bulb_2_PIN, OUTPUT);
  pinMode(Relay_Light_Bulb_3_PIN, OUTPUT);
  
  digitalWrite(on_Board_LED, LOW);
  digitalWrite(Relay_Light_Bulb_1_PIN,LOW);
  digitalWrite(Relay_Light_Bulb_2_PIN, LOW);
  digitalWrite(Relay_Light_Bulb_3_PIN, LOW);
  delay(500);

  //----------------------------------------Set Wifi to STA mode.
  Serial.println();
  Serial.println("-------------Set Wifi to STA mode");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");
  delay(500);
  //---------------------------------------- 

  //----------------------------------------Connect to Wi-Fi (STA).
  Serial.println();
  Serial.println("-------------Connect to WiFi");
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.disconnect(true);
  
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  
  //:::::::::::::::::: The process of connecting ESP32 with WiFi Hotspot / WiFi Router.
  // The process timeout of connecting ESP32 with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 has not been successfully connected to WiFi, the ESP32 will restart.
  // I made this condition because on my ESP32, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.
  
  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(on_Board_LED, HIGH);
    delay(250);
    digitalWrite(on_Board_LED, LOW);
    delay(250);
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      Serial.println();
      Serial.println("Failed to connect to WiFi. The ESP32 will be restarted.");
      Serial.println("-------------");
      digitalWrite(on_Board_LED, LOW);
      delay(1000);
      ESP.restart();
    }
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("Successfully connected to : ");
  Serial.println(WLAN_SSID);
  Serial.println("-------------");
  digitalWrite(on_Board_LED, LOW);
  //:::::::::::::::::: 
  delay(500);
  //---------------------------------------- 

  dht11.begin();
  
  mqtt.subscribe(&led1);
  mqtt.subscribe(&led2);
  mqtt.subscribe(&led3);
}

//________________________________________________________________________________ 



//________________________________________________________________________________ VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:

  MQTT_Connect();

  //----------------------------------------Get "Toggle Button" data from Adafruit IO (Server) to control Relays (Bulbs).
  Adafruit_MQTT_Subscribe *subscription;
  
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &led1) {
      Serial.println();
      Serial.print(F("Toggle Button Light Bulb 1 : "));
      Serial.println((char *)led1.lastread);
      
      if (!strcmp((char*) led1.lastread, "ON")) {
        digitalWrite(Relay_Light_Bulb_1_PIN, HIGH);
      } else if (!strcmp((char*) led1.lastread, "OFF")) {
        digitalWrite(Relay_Light_Bulb_1_PIN, LOW);
      }
    }

    if (subscription == &led2) {
      Serial.println();
      Serial.print(F("Toggle Button Light Bulb 2 : "));
      Serial.println((char *)led2.lastread);
      
      if (!strcmp((char*) led2.lastread, "ON")) {
        digitalWrite(Relay_Light_Bulb_2_PIN, HIGH);
        Serial.println("LED2 turned on");
      } else if (!strcmp((char*) led2.lastread, "OFF")) {
        digitalWrite(Relay_Light_Bulb_2_PIN, LOW);
        Serial.println("LED2 turned off");
      }
    }
    if (subscription == &led3) {
      Serial.println();
      Serial.print(F("Toggle Button Light Bulb 3 : "));
      Serial.println((char *)led3.lastread);
      
      if (!strcmp((char*) led3.lastread, "ON")) {
        digitalWrite(Relay_Light_Bulb_3_PIN, HIGH);
        Serial.println("LED3 turned on");
      } else if (!strcmp((char*) led3.lastread, "OFF")) {
        digitalWrite(Relay_Light_Bulb_3_PIN, LOW);
        Serial.println("LED2 turned off");
      }
    }
  }
  //---------------------------------------- 

  // Call the "read DHT11()" subroutine to get the temperature and humidity values ​​from the DHT11 sensor.
  read_DHT11();

  //----------------------------------------Sending temperature and humidity data to Adafruit IO (Server).
  Serial.println();
  Serial.println("Sending temperature value...");
  
  if (!temperature.publish(temperature_Val)) {
    Serial.println(F("Failed to send temperature value !"));
  } else {
    Serial.println(F("Sending temperature value successfully."));
  }

  Serial.println();
  Serial.println("Sending humidity value...");
  
  if (!humidity.publish(humidity_Val)) {
    Serial.println(F("Failed to send humidity value !"));
  } else {
    Serial.println(F("Sending humidity value successfully."));
  }
  //---------------------------------------- 
}