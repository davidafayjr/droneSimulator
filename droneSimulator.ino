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
// explanation of this setup can be found at 
// https://github.com/firebase/firebase-arduino/blob/master/examples/FirebaseDemo_ESP8266/README.md
// under configuration 
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
      Serial.print("get lat failed:");
      Serial.println(Firebase.error());
      return;
     }
  float inital_lng = Firebase.getFloat("GeoFire/Rouge One/l/1");
  if (Firebase.failed()){
      Serial.print("get lng failed:");
      Serial.println(Firebase.error());
      return;
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
 /*
  * calculates the proper incriment between to vectors
  */
  
  float delta = b - a;
  float incriment = delta/number_of_steps;
  return incriment;
  
}

void runLeg(float start_lat, float start_lng, float stop_lat,  float stop_lng, int number_of_steps){
  /*
   * simulates a drone transmiting its locaion to our database by taking two sets of corordinates
   * and filling in the path between them with intermediate points.  
   */

  float lat_step = createSteps(start_lat, stop_lat, number_of_steps);
  float lng_step = createSteps(start_lng, stop_lng, number_of_steps);
  float each_lat = start_lat;
  float each_lng = start_lng;

  
  int i;
  for (i = 0; i<number_of_steps; i = i+1){
     each_lat = each_lat + lat_step;
     each_lng = each_lng + lng_step;
     //TODO change this to transmit both points simultaniously
     Firebase.setFloat("GeoFire/Rouge One/l/0", each_lat);
     Firebase.setFloat("GeoFire/Rouge One/l/1", each_lng);
     if (Firebase.success()){
      // Serial.println("Successfully updated Location!");
      bool inNFZ = isThisPointInANoFlyZone(each_lat, each_lng);
      if(inNFZ){
        Serial.println("inside nofly zone");
      }else{
        Serial.println("not inside nofly zone");
      }
     }
     if (Firebase.failed()){
      Serial.print("setting location failed:");
      Serial.println(Firebase.error());
     }
     
     delay(100); 
  }
}

bool isThisPointInANoFlyZone(double lat, double lng){
  /*
   * this will determine if at drone has entered a nofly zone
   * it will get the no flyzones from firebase and the compare the 
   * current point to each of the nofly zone polygons and determine 
   * if the point is inside
   */

  FirebaseObject noFlyZones = Firebase.get("NoFlyZones");
    
    if (Firebase.success()){
      int num_o_zones = noFlyZones.getInt("numberOfZones");
      char space = 32;
      
      for(int z=0; z<num_o_zones; z++){
        char zone_num = (z+49);
        String zone = "zone_";
        String zone_key = zone+zone_num;
        String polygonlist = noFlyZones.getString(zone_key);
        if(pointInsidePolygon(polygonlist, lat, lng)){
          return true;
        }
       }
     }
     if (Firebase.failed()){
      Serial.print("get noflyzonse failed:");
      Serial.println(Firebase.error());
     }
  
  return false;
}

bool pointInsidePolygon(String polygonlist, float lat, float lng){

  int num_o_spaces = countSpaces(polygonlist);
  char space = 32;
  bool inside = false;
  
        
  for(int i=0; i <= num_o_spaces; i++){
    String current_point = getValue(polygonlist, space, i);
    String p1_lat_str = getValue(current_point, ',', 1);
    float p1_lat = p1_lat_str.toFloat();
    
    String p1_lng_str = getValue(current_point, ',', 0);
    float p1_lng = p1_lng_str.toFloat();
    
    String next_point = getValue(polygonlist, space, (i+1)%num_o_spaces);
    String p2_lat_str = getValue(next_point, ',', 1);
    float p2_lat = p2_lat_str.toFloat();
    
    String p2_lng_str = getValue(next_point, ',', 0);
    float p2_lng = p2_lng_str.toFloat();
   
    
    inside = lineIntersect(inside, lng, lat, p1_lng, p1_lat, p2_lng, p2_lat);
  }
  return inside;       
}

bool lineIntersect(bool inside, const float x, const float y, const float p1_x, const float p1_y, const float p2_x, const float p2_y){
  float min_y = std::min(p1_y, p2_y);
  
  if (y > min_y){
    float max_y = std::max(p1_y, p2_y);
    if (y <= max_y){
      float max_x = std::max(p1_x, p2_x);
      if(x <= max_x){
        if(p1_y != p2_y){
          float xinters = (y-p1_y)*(p2_x-p1_x)/(p2_y-p1_y)+ p1_x;
          if((p1_x==p2_x)or(x<=xinters)){
            inside = !inside; 
          }
        }
      }
    }
  }
  return inside;
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

int countSpaces(String string){

  int number_of_spaces = 0;
  int i;
  char space = 32;
  
  for (i = 0; i < string.length(); i++){
    char this_char = string[i];
//    Serial.println(this_char);
    if (this_char == space){
      number_of_spaces++;
    }
  }
  return number_of_spaces;
}


