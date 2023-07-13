//KRX94-2050 read by ESP32 
#include <Wire.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include <WiFiUdp.h>
//#include <HTTPClient.h>

// Import the InfluxDB libraries
#include <InfluxDbClient.h>
//#include <InfluxDbCloud.h>

//KRX94-2050 and ESP32 settings
const int PIN_ADC_0 = 32;
const int PIN_ADC_1 = 33;
const int PIN_ADC_2 = 34;
const float ADC_Drange = 4096.0;
const float ADC_Arange = 3600.0;//mV

// cal factors
const float cal_a = -1.16237;
const   float cal_b = 109.723;
const   float cal_c = -1.14131;
const    float cal_d = 108.227;
const float corr_x=3.0;
const float corr_y=2.25;

// Influxdb setting
#define INFLUXDB_IPWITHPORT "http://10.37.0.216:8086"
#define INFLUXDB_DATABASE "cn1" // specify dbtag 
const char* INFLUXDB_USER = "root";
const char* INFLUXDB_PASSWORD = "root";
#define MEASUREMENT "bevel"
#define SENSOR_ID "Bevel-01"

//InfluxDBClient client;

// wifi setting
const char* ssid = "pproomg";
const char* password = "kobeppphysics";

void setup() {
  Serial.begin(115200);
    WiFiSetup();
}

  
void WiFiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("WiFi connection is ready.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

 
void loop() {  
  // gravity (m/s^2)
  float ms2 = 9.80665;
  
  // ofset voltage(0G = 2.5V = 2500mV)
  float offset_voltage = 2500.0;
 
  // get XYZ voltages(mV) 
  // original keep this for calibration
  float x =  (analogRead(PIN_ADC_0) / 1024.0) * 5.0 * 1000;
  float y =  (analogRead(PIN_ADC_1  ) / 1024.0) * 5.0 * 1000;
  float z =  (analogRead(PIN_ADC_2) / 1024.0) * 5.0 * 1000;

  // voltage correction
  float Vx =  (analogRead(PIN_ADC_0) / ADC_Drange) * ADC_Arange- offset_voltage;
  float Vy =  (analogRead(PIN_ADC_1  ) / ADC_Drange) * ADC_Arange- offset_voltage;
  float Vz =  (analogRead(PIN_ADC_2) / ADC_Drange) * ADC_Arange- offset_voltage;
 
  // offset adjustment
  x = x - offset_voltage;
  y = y - offset_voltage;
  z = z - offset_voltage;
 
  // get gravities XYZ
  float xg = x / 1000.0;
  float yg = y / 1000.0;
  float zg = z / 1000.0;

  float Vxg = Vx / 1000.0*ms2;
  float Vyg = Vy / 1000.0*ms2;  
  float Vzg = Vz / 1000.0*ms2;
  
 Serial.print(Vxg);    Serial.print("\t");
 Serial.print(Vyg);    Serial.print("\t");
 Serial.print(Vzg);    Serial.print("\n");
 postToInfluxdb(Vxg,Vyg,Vzg,(xg * ms2 -cal_b)/cal_a+corr_x, (yg * ms2 -cal_d)/cal_c+corr_y);    
  delay(1000);
}

bool postToInfluxdb(float xg,float yg, float zg, float xdeg,float ydeg){    
    //connection  
  InfluxDBClient client;
  client.setConnectionParamsV1(INFLUXDB_IPWITHPORT, INFLUXDB_DATABASE, INFLUXDB_USER, INFLUXDB_PASSWORD);
  Serial.print(client.validateConnection());
  while(client.validateConnection()){
    //retry
    Serial.print(".");
    client.setConnectionParamsV1(INFLUXDB_IPWITHPORT, INFLUXDB_DATABASE, INFLUXDB_USER, INFLUXDB_PASSWORD);       
   delay(1000);    
      }
  Serial.print(" connection to ");
  Serial.print(client.getServerUrl());  
  Serial.print(" was confirmed.");  

  Point pointDevice(MEASUREMENT); // create a new measurement point (the same point can be used for Cloud and v1 InfluxDB);
  // add tags to the datapoints so you can filter them
  pointDevice.addTag("device", SENSOR_ID);
  // Add data fields (values)
  pointDevice.addField("xg", xg);  
  pointDevice.addField("yg", yg);  
  pointDevice.addField("zg", zg);  
  pointDevice.addField("xdeg", xdeg);
  pointDevice.addField("ydeg", ydeg);
  //pointDevice.addField("uptime", millis()); // in addition send the uptime of the Arduino
  Serial.println(client.writePoint(pointDevice)); // returns true if success, false otherwise
  delay(1000);
  return true;
}
