//
// Copyright 2015 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// FirebaseDemo_ESP8266 is a sample that demo the different functions
// of the FirebaseArduino API.

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

// Set these to run example.
#define FIREBASE_HOST "myfirstmapboxapp-11599.firebaseio.com"
#define FIREBASE_AUTH "jdi5ilRiQjD1QkT2zENBBOpex53NhqKBPyCNkMKO"
#define WIFI_SSID "ATT7LyA4Ef"
#define WIFI_PASSWORD "5wyp7d8=yea9"

void setup() {
  Serial.begin(115200);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
 }
void loop() {
  // put your main code here, to run repeatedly:
  float inital_lat = Firebase.getFloat("GeoFire/Rouge One/l/0");
  if (Firebase.failed()){
      Serial.print("setting location failed:");
      Serial.println(Firebase.error());
     }
  float inital_lng = Firebase.getFloat("GeoFire/Rouge One/l/1");
  if (Firebase.failed()){
      Serial.print("setting location failed:");
      Serial.println(Firebase.error());
     }
  float lat_a = 39.70036;
  float lng_a = -83.74279;
  float lat_b = 39.70152;
  float lng_b = -83.73981;
  float lat_c = 39.702135;
  float lng_c = -83.74143;
  float lat_d = 39.700967;
  float lng_d = -83.74357;
  
  runLeg(inital_lat, inital_lng, lat_a, lng_a, 45);
  runLeg(lat_a, lng_a, lat_b, lng_b, 45);
  runLeg(lat_b, lng_b, lat_c, lng_c, 45);
  runLeg(lat_c, lng_c, lat_d, lng_d, 45);
  runLeg(lat_d, lng_d, inital_lat, inital_lng, 45);
  
  delay(6000);

  

}

float createSteps(float a, float b, int number_of_steps){
 
  
  float delta = b - a;
  float incriment = delta/number_of_steps;
  return incriment;
  
}

void runLeg(float start_lat, float start_lng, float stop_lat,  float stop_lng, int number_of_steps){

  float lat_step = createSteps(start_lat, stop_lat, number_of_steps);
  float lng_step = createSteps(start_lng, stop_lng, number_of_steps);
  float each_lat = start_lat;
  float each_lng = start_lng;

  
  int i;
  for (i = 0; i<number_of_steps; i = i+1){
     each_lat = each_lat + lat_step;
     each_lng = each_lng + lng_step;
//     String each_lat_srt = String(each_lat, 10);
//     String each_lng_srt = String(each_lng, 10);
     Serial.print("Lat: ");
     Serial.println(each_lat,6);
     Serial.print("Lng: ");
     Serial.println(each_lng,6);
     Firebase.setFloat("GeoFire/Rouge One/l/0", each_lat);
     //Firebase.setString("GeoFire/Rouge One/l/0", each_lat_srt);
     //Firebase.setString("GeoFire/Rouge One/l/1", each_lng_srt);
     Firebase.setFloat("GeoFire/Rouge One/l/1", each_lng);
     if (Firebase.success()){
      Serial.println("Successfully updated Location!");
     }
     if (Firebase.failed()){
      Serial.print("setting location failed:");
      Serial.println(Firebase.error());
     }
     delay(100); 
  }
}


