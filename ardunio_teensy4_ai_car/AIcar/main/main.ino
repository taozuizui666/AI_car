#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "control.h"
#include "BT_control.h"
#include "control_slide.h"
#include "SD_card.h"
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
#define bufsize     56       // SD_card's buf size
#define V_car       200       // velocity of the car
#define Sensi       1         // sensitivity of control ; bigger than more sensitive

//-------------bluetooth----------------
// connect bluetooth to the board
SoftwareSerial bluetooth(0,1);  // bluetooth's pin RX,TX
int receive_bt = 0;             // data received from bluetooth
//-------------SD_card------------------
// attention ! File is not FILE
File f;
//-------------lidar_data---------------
uint8_t lidar_data = 0;
uint8_t lidar_buf[bufsize];
int     buf_index = 0;

// millis time
unsigned long start_time;
const unsigned long runtime = 5*60*1000;
#define method 2

void setup() {
  start_time = millis();
  //-------------Lidar---------------------
  // connect RX2/TX2 to lidar's RX/TX
  Serial2.begin(baud2);
  // turn on the lidar to receive data
  // attention! lidar can only receive data when it is spinning
  Serial2.write(0xA5);
  Serial2.write(0x20);  
  //-------------pin outputs---------------
  pinMode(Lidar_ctr, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(LEFT_MOTO,OUTPUT);
  pinMode(RIGHT_MOTO,OUTPUT); 

  // control the spin speed of lidar
  analogWrite(Lidar_ctr, 255);  
  //-------------bluetooth----------------
  bluetooth.begin(baud);  // bluetooth's 波特率
  //-------------SD_card------------------
  SD_module_init(CS);     // initialize the SD_module and create Folder 
  #if method==1
  if(!SD.exists("Database/data1.txt"))
  {
    f = SD.open("Database/data1.txt",FILE_WRITE);
    if(f){
      f.close();
    }
  }
  #elif method==2
  if(!SD.exists("Database/data2.txt"))
  {
    f = SD.open("Database/data2.txt",FILE_WRITE);
    if(f){
      f.close();
    }
  } 
  #else
  #endif 
  //-------------end of setup()-----------
}

void loop() {
  if(millis()-start_time < runtime){
  //-------------bluetooth----------------
  // use bluetooth to control the car
  if(bluetooth.available())
  {
    receive_bt = bluetooth.read();
    // bluetooth_control(receive_bt,LEFT_MOTO,RIGHT_MOTO);
    slide_control(receive_bt,V_car,LEFT_MOTO,RIGHT_MOTO,Sensi);
  }
  //-------------SD_card && lidar------------------
  #if method==1
  while(Serial2.available()>0){
    lidar_data = Serial2.read();
    lidar_buf[buf_index++] = lidar_data;
    if(buf_index>=bufsize){
      f = SD.open("Database/data1.txt",FILE_WRITE);
      if(f){
        f.write(lidar_buf,bufsize);
        f.close();
      }
      buf_index = 0;
    }
  }
  // //---------------------------
  // // test how many data received
  //   analogWrite(Lidar_ctr, 0);  
  //   while(1);
  // //---------------------------
  
  #elif method==2
  // f = SD.open("Database/data2.txt",FILE_WRITE);
  // if(Serial2.available()){
  //   lidar_data = Serial2.read();
  //   if(f){
  //     f.print(lidar_data);
  //     f.print(",");
  //     f.close();
  //   }
  // }
  while(Serial2.available() > 0){
      lidar_buf[buf_index++] = Serial2.read();
      
      if(buf_index >= bufsize){   // 缓冲区满
        f = SD.open("Database/data2.txt", FILE_WRITE);
        if(f){
          for(int i=0;i<bufsize;i++){
            f.print(lidar_buf[i]);
            f.print(",");
          }
          f.close();
        }
        buf_index = 0; // 重置下标
      }
  }

  #else
  #endif
  //millis
  }
  else{
    analogWrite(Lidar_ctr, 0);  
    while(1);
  }
  // delay(500);  //prevent receive too fast(can be removed)

  //--------------end of loop()-------------
}
