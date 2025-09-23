// #include <SoftwareSerial.h>
// #include <SPI.h>
// #include <SdFat.h>
// #include <DMAChannel.h>
// #include "control_slide.h"
// // ports
// #define Lidar_TX    7   //teensy RX2
// #define Lidar_RX    8   //teensy TX2
// #define Lidar_ctr   9   //control the spin speed of lidar
// #define CS          10  //chip select(from SD_card module)
// #define MOSI        11
// #define MISO        12
// #define SCK         13  //serial clk(from SD_card module)
// #define LEFT_MOTO   14
// #define RIGHT_MOTO  15
// // paras
// #define baud        115200
// #define baud2       115200
// #define V_car       200       // velocity of the car
// #define Sensi       1         // sensitivity of control ; bigger than more sensitive
// //
// #define bufsize             128       // SD_card's buf size
// #define Lidar_head_num      7     //A5 5A 05 00 00 40 81 
// #define Lidar_data_bin_num  5
// #define num_of_round        8   // the number of data after a round

// //-------------bluetooth----------------
// //// connect bluetooth to the board
// SoftwareSerial bluetooth(0,1);  // bluetooth's pin RX,TX
// int receive_bt = 0;             // data received from bluetooth
// //-------------SD_card------------------
// ////attention ! File is not FILE
// // File f;
// SdFat SD;
// int buf_index = 0;
// unsigned char   BUFFER[bufsize];

// //-------------lidar_data---------------
// bool Lidar_head_message;
// bool new_start;                 // a round is finished
// int  Lidar_list_index;
// unsigned char   Lidar_data;
// unsigned char   Lidar_init[Lidar_head_num];
// unsigned char   Lidar_5byte[Lidar_data_bin_num];
// float   Lidar_list[num_of_round]; //float
// uint16_t  angle_raw;
// uint16_t  dist_raw;
// float     angle_real;
// float     dist_real;
// ////---------------DMA------------
// DMAChannel dma; //activate DMA
// unsigned char Lidar_bufferA[bufsize];
// unsigned char Lidar_bufferB[bufsize];

// volatile bool dma_doneA = false;
// volatile bool dma_doneB = false;

// void dma_is_ready(){
//   dma.clearInterrupt();
//   if(dma.destinationAddress()==(void*)Lidar_bufferA){
//     dma_doneA = true;
//     dma.destinationBuffer(Lidar_bufferB,bufsize);
//   }else{
//     dma_doneB = true;
//     dma.destinationBuffer(Lidar_bufferA,bufsize);
//   }
//   dma.enable();
// }


// void setup(){

//   SPI.begin();
//   // SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));  
//   //-------------Lidar---------------------
//   //// connect RX2/TX2 to lidar's RX/TX
//   Serial2.begin(baud2);
//   //// turn on the lidar to receive data
//   //// attention! lidar can only receive data when it is spinning
//   Serial2.write(0xA5);
//   Serial2.write(0x20);  
//   //-------------pin outputs---------------
//   pinMode(Lidar_ctr, OUTPUT);
//   pinMode(CS, OUTPUT);
//   pinMode(MOSI, OUTPUT);
//   pinMode(SCK, OUTPUT);
//   pinMode(LEFT_MOTO,OUTPUT);
//   pinMode(RIGHT_MOTO,OUTPUT); 

//   //// control the spin speed of lidar
//   analogWrite(Lidar_ctr, 255);  
//   //-------------bluetooth----------------
//   bluetooth.begin(baud);  // bluetooth's 波特率
//   //-------------SD_card------------------
//   SD.begin(CS,SD_SCK_MHZ(16));     // initialize the SD_module and create Folder  
//   //-------------DMA--------------------
//   dma.source(*(volatile uint32_t*)&LPUART4_DATA);
//   dma.destinationBuffer(Lidar_bufferA,bufsize);
//   dma.triggerAtHardwareEvent(DMAMUX_SOURCE_LPUART4_RX);
//   dma.attachInterrupt(dma_is_ready);
//   dma.interruptAtCompletion();

//   dma.enable();
//   //------------------------------------
//   //------------test lidar's data--------
//   Serial.begin(baud);
//   Lidar_head_message=0;
//   //-------------end of setup()-----------

// }

// void loop() { 
//   if(dma_doneA){
//     for (int i = 0; i < 16; i++) {   // 打印前 16 个字节看看
//       Serial.print(Lidar_bufferA[i], HEX);
//       Serial.print(" ");
//     }
//     Serial.println(" ");
//     dma_doneA = false;
//     // dma.destinationBuffer(Lidar_buffer,bufsize);
//     // dma.enable();
//   }
//   if(dma_doneB){
//     for (int i = 0; i < 16; i++) {   // 打印前 16 个字节看看
//       Serial.print(Lidar_bufferB[i], HEX);
//       Serial.print(" ");
//     }
//     Serial.println(" ");
//     dma_doneB = false;    
//   }

//   //--------------end of loop()-------------
// }
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SdFat.h>
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

#define bufsize             128       // SD_card's buf size
#define Lidar_head_num      7     //A5 5A 05 00 00 40 81 
#define Lidar_data_bin_num  5
#define num_of_round        8   // the number of data after a round

//-------------bluetooth----------------
SoftwareSerial bluetooth(0,1);  // bluetooth's pin RX,TX
int receive_bt = 0;             // data received from bluetooth

//-------------SD_card------------------
SdFat SD;
int buf_index = 0;
unsigned char BUFFER[bufsize];

//-------------lidar_data---------------
bool Lidar_head_message;
bool new_start;                 // a round is finished
int  Lidar_list_index;
unsigned char   Lidar_data;
unsigned char   Lidar_init[Lidar_head_num];
unsigned char   Lidar_5byte[Lidar_data_bin_num];
float   Lidar_list[num_of_round]; //float
uint16_t  angle_raw;
uint16_t  dist_raw;
float     angle_real;
float     dist_real;

//---------------DMA------------
DMAChannel dma; //activate DMA
unsigned char Lidar_bufferA[bufsize];
unsigned char Lidar_bufferB[bufsize];

volatile bool dma_doneA = false;
volatile bool dma_doneB = false;
volatile bool bufferA_active = true;

void dma_is_ready(){
  dma.clearInterrupt();
  
  if(bufferA_active){
    dma_doneA = true;
    dma.destinationBuffer(Lidar_bufferB,bufsize);
    bufferA_active = false;
  } else {
    dma_doneB = true;
    dma.destinationBuffer(Lidar_bufferA,bufsize);
    bufferA_active = true;
  }
}

void setup(){
  
  // 首先初始化Serial用于调试
  Serial.begin(baud);
  delay(2000);  // 等待串口稳定
  Serial.println("=== Teensy 4.0 Lidar System Starting ===");

  SPI.begin();
  
  //-------------Lidar---------------------
  Serial.println("Initializing Serial2...");
  Serial2.begin(baud2);
  delay(100);
  
  // 发送启动命令给激光雷达
  Serial.println("Sending start command to Lidar...");
  Serial2.write(0xA5);
  Serial2.write(0x20);  
  
  //-------------pin outputs---------------
  Serial.println("Setting up pins...");
  pinMode(Lidar_ctr, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(LEFT_MOTO, OUTPUT);
  pinMode(RIGHT_MOTO, OUTPUT); 

  // 控制激光雷达转速
  Serial.println("Starting Lidar motor...");
  analogWrite(Lidar_ctr, 255);  
  
  //-------------bluetooth----------------
  Serial.println("Initializing Bluetooth...");
  bluetooth.begin(baud);
  
  //-------------SD_card------------------
  Serial.println("Initializing SD card...");
  if (!SD.begin(CS, SD_SCK_MHZ(16))) {
    Serial.println("SD card initialization failed!");
  } else {
    Serial.println("SD card initialized successfully");
  }
  
  //-------------DMA--------------------
  Serial.println("Setting up DMA...");
  
  // 修正DMA配置 - 对于Teensy 4.0的Serial2
  dma.source((volatile uint8_t&)LPUART4_DATA);  // Serial2对应LPUART6
  dma.destinationBuffer(Lidar_bufferA,bufsize);
  dma.transferSize(1);
  dma.transferCount(bufsize);
  dma.triggerAtHardwareEvent(DMAMUX_SOURCE_LPUART4_RX);
  dma.attachInterrupt(dma_is_ready);
  dma.interruptAtCompletion();
  dma.enable();
  
  Serial.println("DMA configured and enabled");
  
  //------------------------------------
  Lidar_head_message = false;
  new_start = false;
  
  Serial.println("=== Setup Complete ===");
  Serial.println("Waiting for Lidar data...");
}

void loop() { 
  
  // 添加基本的串口输出测试
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {  // 每秒打印一次状态
    Serial.print("Loop running... Time: ");
    Serial.print(millis());
    Serial.print(", DMA A done: ");
    Serial.print(dma_doneA);
    Serial.print(", DMA B done: ");
    Serial.println(dma_doneB);
    lastPrint = millis();
  }
  
  // 检查是否有Serial2数据可用（用于调试）
  // if (Serial2.available()) {
  //   Serial.print("Serial2 data available: ");
  //   while (Serial2.available()) {
  //     Serial.print(Serial2.read(), HEX);
  //     Serial.print(" ");
  //   }
  //   Serial.println();
  // }
  
  // 处理DMA缓冲区A
  if(dma_doneA){
    Serial.println("=== DMA Buffer A Ready ===");
    for (int i = 0; i < 16; i++) {
      if (Lidar_bufferA[i] < 0x10) Serial.print("0");  // 补零显示
      Serial.print(Lidar_bufferA[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    dma_doneA = false;
  }
  
  // 处理DMA缓冲区B
  if(dma_doneB){
    Serial.println("=== DMA Buffer B Ready ===");
    for (int i = 0; i < 16; i++) {
      if (Lidar_bufferB[i] < 0x10) Serial.print("0");  // 补零显示
      Serial.print(Lidar_bufferB[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    dma_doneB = false;    
  }

  // 处理蓝牙数据
  if (bluetooth.available()) {
    receive_bt = bluetooth.read();
    Serial.print("Bluetooth received: ");
    Serial.println(receive_bt);
  }
}