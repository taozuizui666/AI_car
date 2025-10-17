#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <DMAChannel.h> 
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
#define baud        115200
#define baud2       115200
#define V_car       200       // velocity of the car
#define Sensi       1         // sensitivity of control ; bigger than more sensitive
#define bufsize     1024       // SD_card's buf size
#define Lidar_head_num      7     //A5 5A 05 00 00 40 81 
#define Lidar_data_bin_num  5
#define num_of_round        400   // the number of data after a round

//-------------bluetooth----------------
//// connect bluetooth to the board
SoftwareSerial bluetooth(0,1);  // bluetooth's pin RX,TX
int receive_bt = 0;             // data received from bluetooth
//-------------SD_card------------------
////attention ! File is not FILE
File f;
#define FILE_NAME   "data2.txt"
// auto f;
// SdFat SD;
int buf_index = 0;
//-------------lidar_data---------------
bool Lidar_head_message;
int  Lidar_list_index;
unsigned char   Lidar_data;
unsigned char   Lidar_init[Lidar_head_num];
unsigned char   Lidar_5byte[Lidar_data_bin_num];
float           Lidar_list_ang[num_of_round]; //*Lidar_data_bin_num
float           Lidar_list_dis[num_of_round]; //*Lidar_data_bin_num

uint16_t     angle_raw;
uint16_t     dist_raw;
float        angle_real;
float        dist_real;
//----------------DMA-----------------------
DMAChannel dma;
volatile unsigned char  rx_buffer[1024];

void dmaComplete() {
  Serial.println("DMA buffer full - processing data");
}

void setup() { 
  //-------------Lidar---------------------
  //// connect RX2/TX2 to lidar's RX/TX
  Serial2.begin(baud2);
  //// turn on the lidar to receive data
  //// attention! lidar can only receive data when it is spinning
  Serial2.write(0xA5);
  Serial2.write(0x20);  
  //-------------pin outputs---------------
  pinMode(Lidar_ctr, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(LEFT_MOTO,OUTPUT);
  pinMode(RIGHT_MOTO,OUTPUT); 
  //// control the spin speed of lidar
  analogWrite(Lidar_ctr, 255);  
  //-------------bluetooth----------------
  bluetooth.begin(baud);  // bluetooth's 波特率 
  //------------test lidar's data--------
  Serial.begin(baud);
  //-------------SD_card------------------
  SD.begin(CS);
  //-------------end of setup()-----------
  Lidar_head_message = 0;
  Lidar_list_index = 0;
  //----------------------DMA----------------------
  dma.begin(true);
  dma.source((volatile uint8_t&)LPUART4_DATA);
  dma.transferSize(1);
  dma.destination(rx_buffer[0]);
  dma.destinationCircular(rx_buffer, 1024);
  dma.triggerAtHardwareEvent(DMAMUX_SOURCE_LPUART4_RX);
  dma.interruptAtCompletion();
  dma.attachInterrupt(dmaComplete);
  dma.enable();
}

void loop() {   
  //--------------end of loop()-------------
}