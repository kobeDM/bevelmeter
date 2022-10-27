//KRX94-2050 read by ESP32 
#include <Wire.h>
#include <WiFi.h>
#include <ESPmDNS.h>

const int PIN_ADC_0 = 32;
const int PIN_ADC_1 = 33;
const int PIN_ADC_2 = 34;


void setup() {
  Serial.begin(115200);
}
 
void loop() {
  float cal_a = -1.16237;
  float cal_b = 109.723;
  float cal_c = -1.14131;
   float cal_d = 108.227;



  
  // 地球の重力である1Gの加速度(m/s^2)
  float ms2 = 9.80665;
  
  // 電源電圧5V時のオフセット電圧(0G = 2.5V = 2500mV)
  float offset_voltage = 2500.0;
 
  // XYZの電圧(mV)を取得する
  float x =  (analogRead(PIN_ADC_0) / 1024.0) * 5.0 * 1000;
  float y =  (analogRead(PIN_ADC_1  ) / 1024.0) * 5.0 * 1000;
  float z =  (analogRead(PIN_ADC_2) / 1024.0) * 5.0 * 1000;
 
  // XYZからオフセット電圧を引いた電圧を求める
  x = x - offset_voltage;
  y = y - offset_voltage;
  z = z - offset_voltage;
 
  // XYZから重力を求める
  float xg = x / 1000.0;
  float yg = y / 1000.0;
  float zg = z / 1000.0;
 
  // XYZの重力から加速度(m/s^2)を算出して出力する
//  Serial.print("x : ");
  Serial.print(xg * ms2);
    Serial.print("\t");
//  Serial.print(" y : ");
  Serial.print(yg * ms2);
    Serial.print("\t");
//  Serial.print(" z : ");
  Serial.print(zg * ms2);
//  Serial.println(" m/s^2");
//     Serial.print("\n");

    Serial.print("\t");
  Serial.print((xg * ms2 -cal_b)/cal_a);
    Serial.print("\t");
      Serial.print((yg * ms2 -cal_d)/cal_c);
          Serial.print("\n");
  delay(1000);
}
