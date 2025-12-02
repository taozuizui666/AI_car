#include <SoftwareSerial.h>
#include <RPLidar.h>
#include <SPI.h>
#include <SD.h>
#include "control_slide.h"
// ports

#define Lidar_ctr     9   //control the spin speed of lidar
#define CS            4  //chip select(from SD_card module)

#define SCK         13  //serial clk(from SD_card module)

  #define LEFT_MOTO   14
  #define RIGHT_MOTO  15

// paras
#define bluetooth_baud      9600
#define serial_baud         115200
#define lidar_baud          115200
#define V_car               70       // velocity of the car
#define Sensi               1         // sensitivity of control ; bigger than more sensitive

#define LIDAR_RESOLUTION    200
#define time_of_round       160     ////-------------------about 147 ms a round------------------
// #define num_of_round        250     // the number of data after a round   250
// #define NUM_of_yield        50      // how many rounds to yield
// #define NUM_to_store        50      // how many rounds to store into SD

//-------------bluetooth----------------
SoftwareSerial bluetooth(2, 3);  // Rx, Tx
bool is_recording = false;

int receive_bt = -1;              // data received from bluetooth
#define SD_start_store    255
bool SD_store  = 0;
//-------------SD_card------------------
////attention ! File is not FILE
File f;
#define FILE_NAME   "sq.txt"
//-------------Lidar--------------------
RPLidar lidar;
uint16_t distances[LIDAR_RESOLUTION];


long last_time = 0;

void setup() {  

  for (int a = 0; a < LIDAR_RESOLUTION; a++) {
    distances[a] = 0;
  }

  lidar.begin(Serial);

  //-------------pin outputs---------------
  pinMode(Lidar_ctr, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(7, OUTPUT);

  SD.begin(CS);
  bluetooth.begin(9600);

  rplidar_response_device_info_t info;
  if (IS_OK(lidar.getDeviceInfo(info, 100))) {
      lidar.startScan();
      analogWrite(Lidar_ctr, 255);
      delay(1000);
  }
  last_time = millis();
  //---------------end of setup------------------
}

void loop() {
  // //-------------bluetooth 控制与文件管理----------------
  if(bluetooth.available()) {
    receive_bt = bluetooth.read();    
    if(receive_bt == 255) {
      if (!is_recording) {
        f = SD.open(FILE_NAME, FILE_WRITE);
        if (f) {
          is_recording = true;
        } 
      }
      digitalWrite(7, HIGH);
    } 
    else if(receive_bt == 200) {
      if (is_recording) {
        is_recording = false;
        f.close();
      }
      digitalWrite(7, LOW);
    }    
    slide_control(receive_bt, V_car, LEFT_MOTO, RIGHT_MOTO, Sensi);
  }

  // //---------------store into SD----------------------

  while(millis() - last_time <= time_of_round){
    lidar.waitPoint();
    uint16_t dist = (uint16_t)lidar.getCurrentPoint().distance;
    uint16_t angl = (uint16_t)lidar.getCurrentPoint().angle;
    // uint16_t qual = (uint16_t)lidar.getCurrentPoint().quality;
    
    int index = (int)((angl / 360.0f) * (float)LIDAR_RESOLUTION);

    if (dist<10000) {
      distances[index] = (uint16_t)dist;
    }else{
      // distances[index] = 0;
    }
  }

  if(is_recording==true){
    for(int i=0;i<LIDAR_RESOLUTION;i++) {
      f.print(distances[i]);
      f.print(',');
    }
    f.println(receive_bt);
  }else {}
  while (Serial.available()) {
    Serial.read();
  }
  last_time = millis();

}