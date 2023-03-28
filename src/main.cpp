#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include "time.h"

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

const char* ssid = "PoleDeVinci_DVIC";
const char* password = "8PfURsp!dvic";

// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "http://172.21.72.145:8086"
// InfluxDB v2 server or cloud API token (Use: InfluxDB UI -> Data -> API Tokens -> Generate API Token)
#define INFLUXDB_TOKEN "5jfcePpPUG7T3I_dTRIcc7ZOCiqmh7oC01LoYPYMLZ2MigDw4rJ6qfC9dGchb3yxeF74TEWFsK77RZSFMPvSrw=="
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "dvic"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "évolution_plantes"

//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
Point sensor("intensité_lumineuse");

float lux = 0;
#define AnalogPin 33


// //to get time
const char* ntpServer="pool.ntp.org";
// //we choose a time zone
const long gmtOffset_sec=3600;
// //we choose the daylight saving time
const int daylightOffset_sec = 3600;

// // How many leds in your strip?
const int led=198;
#define NUM_LEDS 198
// // For led chips like WS2812, which have a data line, ground, and power, you just
// // need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// // ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// // Clock pin only needed for SPI based chipsets when not using hardware SPI (CLOCK_PIN 13 si utile)
#define DATA_LEDS_PIN 4




// // Define the array of leds
CRGB leds[NUM_LEDS];


void setup(){
  Serial.begin(115200);

  //WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());


//On se déconnecte
  //WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);

//     // ## Clockless types ##
  FastLED.addLeds<NEOPIXEL, DATA_LEDS_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed


  //INFLUXDB_SETUP
  // Add tags
  sensor.addTag("device", "ESP32");
  sensor.addTag("SSID", WiFi.SSID());

  pinMode(AnalogPin, INPUT);
  

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

}

void printLocalTime(tm timeinfo){
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
 Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  /*Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");*/
}

 // bool start = false;

void loop() 
{ 
  lux= 3000 - analogRead(AnalogPin);
  

    //INFLUXDB_LOOP
  // Clear fields for reusing the point. Tags will remain untouched
  sensor.clearFields();

  // Store measured value into point
  // Report RSSI of currently connected network
  sensor.addField("lumen", lux);

  
  struct tm timeinfo;

  //Actualisation with the internal time of the ESP
  getLocalTime(&timeinfo);
  printLocalTime(timeinfo);
  
  //We register Hour, Minutes, Secondes
  int myMIN=timeinfo.tm_min;
  int myH=timeinfo.tm_hour;
  int myS=timeinfo.tm_sec;

  //print on the console for the TIME
  Serial.print(myH);
  Serial.print(":");
  Serial.print(myMIN);
  Serial.print(":");
  Serial.println(myS);

  Serial.print("Luminosité ambiente : ");
  Serial.println(lux);
  
  delay(1000);

  if (myMIN>=0 && myMIN<15) //between 0 and 15 minutes we switch the leds off
  {
    FastLED.setBrightness(0);
    sensor.addField("brightness", 0);
    for(int i=0;i<=198;i++)
    {
    leds[i]=CRGB::Black;     
    }
  }

  if (myMIN>=15 && myMIN<=59) //between 15 and 59 minutes we switch the leds on with a brightness depending on the luminosity and a ratio of 16:4 (red:blue)
  {
    float value = (1-lux/3000)*255;
    value = constrain(value, 0, 255);
    FastLED.setBrightness(int(value));
    Serial.print("Valeur des leds: ");
    Serial.println(int(value));
    sensor.addField("brightness", int(value));
    for (int i=0;i<=198;i++)
    {
      if(i%4==0)
      {
        leds[i]=CRGB::Blue;
      }
      else
      {
        leds[i]=CRGB:: Red;
      } 
    }
  }

  FastLED.show();

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

}