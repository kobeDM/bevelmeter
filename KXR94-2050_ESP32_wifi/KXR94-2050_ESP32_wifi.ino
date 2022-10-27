//KRX94-2050 read by ESP32 
#include <Wire.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include <WiFiUdp.h>
#include <HTTPClient.h>

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

// data post setting
#define SENSOR_ID "CN01-Bevel01"
#define DBTAG "/bevel.log" // specify lag 
#define FLUENTD_IP "10.37.0.210"
#define FLUENTD_PORT 8888


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
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

 
void loop() {  
  // 地球の重力である1Gの加速度(m/s^2)
  float ms2 = 9.80665;
  
  // 電源電圧5V時のオフセット電圧(0G = 2.5V = 2500mV)
  // 
  float offset_voltage = 2500.0;
 
  // XYZの電圧(mV)を取得する 
  // original keep this for calibration
  float x =  (analogRead(PIN_ADC_0) / 1024.0) * 5.0 * 1000;
  float y =  (analogRead(PIN_ADC_1  ) / 1024.0) * 5.0 * 1000;
  float z =  (analogRead(PIN_ADC_2) / 1024.0) * 5.0 * 1000;

  // right voltage
  float Vx =  (analogRead(PIN_ADC_0) / ADC_Drange) * ADC_Arange- offset_voltage;
  float Vy =  (analogRead(PIN_ADC_1  ) / ADC_Drange) * ADC_Arange- offset_voltage;
  float Vz =  (analogRead(PIN_ADC_2) / ADC_Drange) * ADC_Arange- offset_voltage;


 
  // XYZからオフセット電圧を引いた電圧を求める
  x = x - offset_voltage;
  y = y - offset_voltage;
  z = z - offset_voltage;
 
  // XYZから重力を求める
  float xg = x / 1000.0;
  float yg = y / 1000.0;
  float zg = z / 1000.0;

  float Vxg = Vx / 1000.0*ms2;
  float Vyg = Vy / 1000.0*ms2;  
  float Vzg = Vz / 1000.0*ms2;
  
 Serial.print(Vxg);    Serial.print("\t");
 Serial.print(Vyg);    Serial.print("\t");
 Serial.print(Vzg);    Serial.print("\n");

 
  // XYZの重力から加速度(m/s^2)を算出して出力する
//  Serial.print(xg * ms2);
//  Serial.print(xg * ms2);
//    Serial.print("\t");
//  Serial.print(" y : ");
//  Serial.print(yg * ms2);
//    Serial.print("\t");
//  Serial.print(" z : ");
//  Serial.print(zg * ms2);
//  Serial.println(" m/s^2");
//     Serial.print("\n");

//    Serial.print("\t");
//  Serial.print((xg * ms2 -cal_b)/cal_a);
//    Serial.print("\t");
//      Serial.print((yg * ms2 -cal_d)/cal_c);
//          Serial.print("\n");

//  postToFluentd(xg * ms2,yg * ms2,zg * ms2,(xg * ms2 -cal_b)/cal_c, (yg * ms2 -cal_d)/cal_c);
  postToFluentd(Vxg,Vyg,Vzg,(xg * ms2 -cal_b)/cal_a+corr_x, (yg * ms2 -cal_d)/cal_c+corr_y);
    
  delay(1000);
}

bool postToFluentd(float xg,float yg, float zg, float xdeg,float ydeg){
  HTTPClient http;
  http.begin(FLUENTD_IP, FLUENTD_PORT, DBTAG);
  delay(1000);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
  char jsonStr[200];
  //sprintf(jsonStr, "{\"sensor_id\": \"%s\", \"load\": %3.3f}", SENSOR_ID, data);

  sprintf(jsonStr, "{\"sensor_id\": \"%s\", \"xg\": %.1f, \"yg\": %.1f, \"zg\": %.1f, \"xdeg\": %.1f, \"ydeg\": %.1f}"
,  SENSOR_ID,  xg , yg,zg,xdeg,ydeg);
  Serial.println(jsonStr);
  int code = http.POST(jsonStr); // POST data to na10
  http.end();
  if (code != HTTP_CODE_OK) {
    return false;
  }
  return true;
}
