#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "control.h"
#include "BT_control.h"
#include "SD_card.h"
// ports
#define CS          10  //chip select(from SD_card module)
#define MOSI        11
#define MISO        12
#define SCK         13  //serial clk(from SD_card module)
#define LEFT_MOTO   14
#define RIGHT_MOTO  15
// paras
#define SCK_MHZ     10  // 10HZ
#define baud        115200

SoftwareSerial bluetooth(0,1);  // bluetooth's pin RX,TX
File f;
int receive_bt = 0;

void setup() {
  //  pin outputs
  pinMode(LEFT_MOTO,OUTPUT);
  pinMode(RIGHT_MOTO,OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  // bluetooth
  bluetooth.begin(baud);  // bluetooth's 波特率
  SD_module_init(CS);     // initialize the SD_module and create Folder 
  // SD.begin(CS)
  // if(!SD.exists("Database"))
  // {
  //   SD.mkdir("Database");
  // }
  if(!SD.exists("Database/data.csv"))
  {
    f = SD.open("Database/data.csv",FILE_WRITE);
    if(f){
      // f.println("ms,sensor");
      f.close();
    }
  }

}

void loop() {
  receive_bt = -1;
  if(bluetooth.available())
  {
    receive_bt = bluetooth.read();
    bluetooth_control(receive_bt,LEFT_MOTO,RIGHT_MOTO);
  }
  f = SD.open("Database/data.csv",FILE_WRITE);
  if(f && receive_bt!= -1){
    f.print(receive_bt);
    f.print(",");
    f.close();
  }
  delay(500);
}
