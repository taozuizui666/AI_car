#include <SoftwareSerial.h>

// #include "control.h"
// #include "BT_control.h"
#include "control_slide.h"
// #include "SD_card.h"
// ports
#define LEFT_MOTO   14
#define RIGHT_MOTO  15
// paras
#define SCK_MHZ     10  // 10HZ
#define baud        115200
#define baud2       115200
#define bufsize     256

#define V_car       205
#define Sensi       1

//-------------bluetooth----------------
// connect bluetooth to the board
SoftwareSerial bluetooth(0,1);  // bluetooth's pin RX,TX
int receive_bt = 0;             // data received from bluetooth

void setup() {

  //-------------pin outputs---------------
  pinMode(LEFT_MOTO,OUTPUT);
  pinMode(RIGHT_MOTO,OUTPUT); 
  //-------------bluetooth----------------
  bluetooth.begin(baud);  // bluetooth's 波特率
  //-------------end of setup()-----------
}

void loop() {
  //-------------bluetooth----------------
  // use bluetooth to control the car
  if(bluetooth.available())
  {
    receive_bt = bluetooth.read();
    slide_control(receive_bt,V_car,LEFT_MOTO,RIGHT_MOTO,Sensi);
  }
  //--------------end of loop()-------------
}
