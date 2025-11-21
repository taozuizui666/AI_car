#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "control_slide.h"
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
#define bluetooth_baud     9600
#define serial_baud        115200
#define lidar_baud         115200
#define V_car       70       // velocity of the car
#define Sensi       1         // sensitivity of control ; bigger than more sensitive
#define Lidar_head_num      7     //A5 5A 05 00 00 40 81 
#define Lidar_data_bin_num  5

#define time_of_round       160     ////-------------------about 147 ms a round------------------
#define num_of_round        250   // the number of data after a round   250

//-------------bluetooth----------------
int receive_bt = -1;             // data received from bluetooth
#define SD_start_store    255
bool SD_store  = 0;
//-------------SD_card------------------
////attention ! File is not FILE
File f;
#define FILE_NAME   "data3.txt"

//-------------lidar_data---------------
bool Lidar_head_message = 0;
unsigned char   Lidar_data;
unsigned char   Lidar_init[Lidar_head_num];
unsigned char   Lidar_5byte[Lidar_data_bin_num];
int             Lidar_list_ang[num_of_round]; //*Lidar_data_bin_num
float           Lidar_list_dis[num_of_round]; //*Lidar_data_bin_num

uint16_t     angle_raw;
uint16_t     dist_raw;
float        angle_real;
float        dist_real;

long last_time = 0;
int num_zero = 0;

void setup() {  
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  //-------------pin outputs---------------
  pinMode(Lidar_ctr, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(LEFT_MOTO,OUTPUT);
  pinMode(RIGHT_MOTO,OUTPUT); 
  //------------test lidar's data--------
  // Serial.begin(serial_baud);
  //-------------SD_card------------------
  SD.begin(CS);
  f = SD.open("data3.txt",FILE_WRITE);
  f.print(0);
  f.print('\n');
  f.flush();
  //// control the spin speed of lidar
  analogWrite(Lidar_ctr, 255); 
  // connect to bluetooth
  Serial1.begin(bluetooth_baud);
  //-------------Lidar---------------------
  //// connect RX2/TX2 to lidar's RX/TX
  Serial2.begin(lidar_baud);
  //// turn on the lidar to receive data
  //// attention! lidar can only receive data when it is spinning
  Serial2.write(0xA5);
  Serial2.write(0x40);    // Reset RPLidar
  delay(3000);
  Serial2.write(0xA5);
  Serial2.write(0x20);    // command RPLidar to spin  
  //-------------end of setup()-----------
}
void loop() {
  //-------------bluetooth----------------
  if(Serial1.available())
  {
    receive_bt = Serial1.read();
    if(receive_bt == 255){
      SD_store = 1;
      digitalWrite(2, HIGH);
    }else if(receive_bt == 200){
      SD_store = 0;
      digitalWrite(2, LOW);      
    }    
    slide_control(receive_bt,V_car,LEFT_MOTO,RIGHT_MOTO,Sensi);
  }
  //--------------------------------------------------------------
  //----------------------lidar_SD--------------------------------
  if((!Lidar_head_message)&&(Serial2.available()>=Lidar_head_num))
  {
    for(int i=0;i<Lidar_head_num;i++)
    {
      Lidar_data = Serial2.read();
    }// release the reading buff, otherwise the message will stay in the buff
    Lidar_head_message = 1;
  }
  //--------------------------------------------------------
  //--------------------------------------------------------
  last_time = millis();
  while(Lidar_head_message)
  {
    digitalWrite(3, HIGH);
    if(Serial2.available()>=Lidar_data_bin_num)
    {
      for(int i=0;i<Lidar_data_bin_num;i++)
      {
        Lidar_5byte[i] = Serial2.read();
      }
      angle_raw = (((uint16_t)Lidar_5byte[2] << 8) | Lidar_5byte[1]) >> 1;
      dist_raw = (((uint16_t)Lidar_5byte[4] << 8) | Lidar_5byte[3]);
      angle_real = angle_raw/64.0;
      dist_real = dist_raw/4.0;
      if(dist_real<3000){
        int index = map(angle_real, 0, 361, 0, num_of_round);
        if(dist_real==0 && index > 0){
          Lidar_list_dis[index] = Lidar_list_dis[index-1];
        }
        Lidar_list_dis[index] = dist_real;
      }
      if(millis()-last_time>time_of_round){
        if(SD_store==1){
          if(f){
            for(int i=0;i<num_of_round;i++){
              f.print(Lidar_list_dis[i]);
              f.print(',');
            }
          }
          f.print('\n');
          f.flush();
        }
        break;
      }      
    }
  }  
  //--------------end of loop()-------------
}
int map(float x, int in_min, int in_max, int out_min, int out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}