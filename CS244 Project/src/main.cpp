#include <Arduino.h>
#include "HX711.h"
#include <inttypes.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <WiFi.h>
#include <HttpClient.h>

// sudo chmod a+rw /dev/ttyACM0
// ssh command: ssh -i "CS244FinalProject.pem" ubuntu@ec2-3-135-216-152.us-east-2.compute.amazonaws.com

// Define HX711 pins
#define DOUT 27  
#define CLK 26  

HX711 scale;
float foodConsumed = 0.0;
float lastDay[86400];
int timeSinceStart = 0;
int delayInterval = 1000;
float lastSteadyWeight = 0;
float lastTenWeights[10];

// Wi-Fi configuration
const char* ssid = "OurPhone";
const char* pass = "number347";
const char* serverIP = "3.135.216.152";
const int serverPort = 5000;

void setup() {
  Serial.begin(9600); 
  scale.begin(DOUT, CLK);
  // Check if the HX711 is connected
  while (!scale.is_ready()) {
    
    Serial.println("Not wired correctly");
  }

  Serial.println("HX711 initialized");
  
  
  
  scale.set_scale(228.f); //Calibrate
  scale.tare();           
  Serial.println("Finished Setup");

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected!");
}

void loop() {
  
  if (scale.is_ready()) {
    float weight = scale.get_units(5); // Take 5 readings for average

    

    Serial.print("Weight: ");
    Serial.print(weight, 2); // Print weight with 2 decimal places
    Serial.println(" g");
    Serial.print("Food consumed weight so far: ");
    Serial.print(foodConsumed, 2);
    Serial.println(" g");

    
    for(int i = 0; i < 10; i++){
      lastTenWeights[i] = scale.get_units();
      delay(delayInterval / 10);
    }
    float minimum = 9999;
    float maximum = 0;
    for(int i = 0; i < 10; i++){
      if (lastTenWeights[i] > maximum){
        maximum = lastTenWeights[i];
      }
      if (lastTenWeights[i] < minimum){
        minimum = lastTenWeights[i];
      }
      
    }
    if (abs(maximum) - abs(minimum) < 0.5 && maximum < 10000){
      // the heavier the weight, the more negative, therefore a weight being heavier is more negative
      // ex now is -5, was -10, means 5 eaten
      // now, only update steady weight if more than 1
      if (abs(maximum - lastSteadyWeight) > 1){
        if (lastSteadyWeight - maximum> 1){
          foodConsumed += lastSteadyWeight - maximum;
        }
        lastSteadyWeight = maximum;
      }
    }
  } else {
    Serial.println("HX711 not ready. Check connections.");
  }


   // Wait 1 second before the next reading
  if((timeSinceStart / 1000) == 86400){
    timeSinceStart = 0;
  }
  timeSinceStart += delayInterval;
  // delay(delayInterval);
  WiFiClient client;
  HttpClient http(client);

  String url = "/?eaten=" + String(foodConsumed) + "&weight=" + String(lastTenWeights[0]);
    Serial.println("Sending HTTP GET request: " + url);

    // Send the HTTP GET request to the server
    int httpStatus = http.get(serverIP, serverPort, url.c_str(), NULL);
    if (httpStatus == 0) {
        Serial.println("HTTP request sent successfully.");
    } else {
        Serial.printf("HTTP request failed with error code: %d\n", httpStatus);
    }

    // Clean up and delay before the next iteration
    http.stop();
}