#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "control_slide.h"
#include "RPLidar.h"
// ports
#define Lidar_TX    7   //teensy RX2
#define Lidar_RX    8   //teensy TX2
#define Lidar_ctr   9   //control the spin speed of lidar
#define CS          10  //chip select(from SD_card module)
#define MOSI        11
#define MISO        12
#define SCK         13  //serial clk(from SD_card module)
#define LEFT_MOTO   14
#define RIGHT_MOTO  15
// paras
#define SCK_MHZ     10  // 10HZ
#define baud        115200
#define baud2       115200
#define V_car       200       // velocity of the car
#define Sensi       1         // sensitivity of control ; bigger than more sensitive
#define bufsize     1024       // SD_card's buf size

#define LIDAR_RESOLUTION  240

//-------------bluetooth----------------
//// connect bluetooth to the board
SoftwareSerial bluetooth(0,1);  // bluetooth's pin RX,TX
int receive_bt = 0;             // data received from bluetooth
//-------------SD_card------------------
////attention ! File is not FILE
// File f;
// int buf_index = 0;
// unsigned char   BUFFER[bufsize];
//-------------lidar_data---------------
RPLidar lidar;
float distances[LIDAR_RESOLUTION];
float lidar_distance = 0;
float lidar_angle = 0;
float lidar_quality = 0;
//--------------time---------------
long last_time = 0;

void setup() {
  //-------------pin outputs---------------
  pinMode(Lidar_ctr, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(LEFT_MOTO,OUTPUT);
  pinMode(RIGHT_MOTO,OUTPUT); 
  //// control the spin speed of lidar
  analogWrite(Lidar_ctr, 255); 
  //-------------Lidar---------------------
  //// connect RX2/TX2 to lidar's RX/TX
  Serial.begin(baud);
  Serial2.begin(baud2);
    Serial.print(9);

  lidar.begin(Serial2);   
  //-------------bluetooth----------------
  bluetooth.begin(baud);  // bluetooth's 波特率
  //-------------SD_card------------------
  SD.begin(CS);     // initialize the SD_module and create Folder 

  for(int i=0;i<LIDAR_RESOLUTION;i++){
    distances[i] = 0;
  }
  lidar.startScan();
  delay(50);

  //------------test lidar's data--------
  Serial.print(0);
  //-------------end of setup()-----------  
}

void loop() {
  Serial.print(1);
    //-------------bluetooth----------------
    //// use bluetooth to control the car
    if(bluetooth.available())
    {
      receive_bt = bluetooth.read();
      slide_control(receive_bt,V_car,LEFT_MOTO,RIGHT_MOTO,Sensi);
      if(receive_bt==200)// when control_signal is "stop",then stop both car and lidar
      {
        lidar.stop();
      }
    }
    //-------------------Lidar---------------------
    lidar.waitPoint();
    lidar_distance = lidar.getCurrentPoint().distance;
    lidar_angle = lidar.getCurrentPoint().angle;
    lidar_quality = lidar.getCurrentPoint().quality;
    if(lidar_distance<2000 && lidar_quality>0){
      int index = mapAndRoundAngle(lidar_angle);
      if(lidar_distance==0){
        distances[index] = distances[index-1];
      }
      distances[index] = lidar_distance;
    }

    Serial.print(2);//distances[0]
    //------------------SD--------------------------
  if ((receive_bt != 200) && (receive_bt != 0)) {
      if ((millis() - last_time) > 200) {
        String toPython = "";
        for (int i = 0; i < LIDAR_RESOLUTION; i++) {
          toPython.concat(distances[i]);
          toPython.concat(",");
        }
        toPython.concat(receive_bt);
        File dataFile = SD.open("datalog.txt", FILE_WRITE);
        if (dataFile) {
          dataFile.println(toPython);
          dataFile.close();
        } 
        last_time = millis();
      }
    }
  //--------------end of loop()-------------
}

int mapAndRoundAngle(float angle) {
  // Subtract 90 degrees to offset the angles clockwise
  angle -= 90;
  // Ensure the angle stays within the valid range (0-360)
  if (angle < 0) {
    angle += 360;
  } else if (angle >= 360) {
    angle -= 360;
  }
  // Map the angle from 0-360 degrees to 0-239 (LIDAR_RESOLUTION)
  int index = int(map(angle, 0, 360, 0, LIDAR_RESOLUTION));
  return index;
}