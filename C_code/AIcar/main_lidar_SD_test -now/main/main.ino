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
#define bluetooth_baud  9600
#define baud        115200
#define baud2       115200
#define V_car       70       // velocity of the car
#define Sensi       1         // sensitivity of control ; bigger than more sensitive
#define bufsize     1024       // SD_card's buf size
#define Lidar_head_num      7     //A5 5A 05 00 00 40 81 
#define Lidar_data_bin_num  5

#define time_of_round       160     ////-------------------about 147 ms a round------------------
#define num_of_round        250   // the number of data after a round

//-------------bluetooth----------------
//// connect bluetooth to the board
SoftwareSerial bluetooth(0,1);  // bluetooth's pin RX,TX
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
  Serial2.begin(baud2);
  //// turn on the lidar to receive data
  //// attention! lidar can only receive data when it is spinning
  Serial2.write(0xA5);
  Serial2.write(0x40);    // Reset RPLidar
  delay(3000);
  Serial2.write(0xA5);
  Serial2.write(0x20);    // command RPLidar to spin
  //-------------bluetooth----------------
  bluetooth.begin(bluetooth_baud);  // bluetooth's 波特率 
  //------------test lidar's data--------
  // Serial.begin(baud);
  //-------------SD_card------------------
  SD.begin(CS);
  //-------------end of setup()-----------
}
void loop() {
  //-------------bluetooth----------------
  if(bluetooth.available())
  {
    receive_bt = bluetooth.read();
    if(receive_bt == SD_start_store){
      SD_store = 1;
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
      // Serial.print(angle_real);
      // Serial.print('\t');
      if(dist_real<3000){
        int index = map(angle_real, 0, 360, 0, num_of_round);
        // Serial.print(index);
        // Serial.print('\t');
        if(dist_real==0){
          Lidar_list_dis[index] = Lidar_list_dis[index-1];
        }
        Lidar_list_dis[index] = dist_real;
      }
      if(millis()-last_time>time_of_round){
        if(SD_store &&(receive_bt != 200)){
          f = SD.open(FILE_NAME,FILE_WRITE);
          for(int i=0;i<num_of_round;i++){
            // f.print('\t');
            // f.print(i);
            // f.print('\t');
            f.print(Lidar_list_dis[i]);
            f.print(',');
            // Serial.print(Lidar_list_dis[i]);
            // Serial.print('\t');
            if(Lidar_list_dis[i]==0.00){num_zero++;}
          }
          // Serial.print('\n');
          // f.print('\t');
          // f.print(num_zero);
          f.print(',');
          f.print(receive_bt);
          f.print('\n');
          f.close();
          num_zero = 0;
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