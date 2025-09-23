#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "control_slide.h"
#include "SD_card.h"

// 选择解决方案：1=严格包头验证, 2=时间戳方式, 3=数据计数方式, 4=原始数据流
#define SOLUTION 2

// ports
#define Lidar_TX    7   
#define Lidar_RX    8   
#define Lidar_ctr   9   
#define CS          10  
#define MOSI        11
#define MISO        12
#define SCK         13  
#define LEFT_MOTO   14
#define RIGHT_MOTO  15

// paras
#define baud        115200
#define baud2       115200
#define bufsize     2048       
#define V_car       200       
#define Sensi       1         

SoftwareSerial bluetooth(0,1);  
int receive_bt = 0;             

File f;
int buf_index = 0;
unsigned char BUFFER[bufsize];

// 通用变量
unsigned long last_round_time = 0;
int packet_count = 0;
int total_bytes_in_round = 0;

#if SOLUTION == 1
//========== 解决方案1: 严格包头验证 ==========
unsigned char expected_header[7] = {0xA5, 0x5A, 0x05, 0x00, 0x00, 0x40, 0x81};
int header_index = 0;
bool header_verified = false;
unsigned char data_packet[5];
int packet_byte_count = 0;
unsigned char round_buffer[2000];  // 存储一圈数据
int round_buffer_index = 0;
bool round_complete = false;

#elif SOLUTION == 2
//========== 解决方案2: 基于时间戳的换行 ==========
#define ROUND_TIMEOUT_MS 200  // RPLiDAR一圈大约200ms
unsigned char temp_buffer[10];
int temp_index = 0;

#elif SOLUTION == 3
//========== 解决方案3: 基于数据包计数 ==========
#define PACKETS_PER_ROUND 60  // 根据RPLiDAR型号调整，一圈大约60个数据包
unsigned char data_packet[5];
int packet_byte_count = 0;

#elif SOLUTION == 4
//========== 解决方案4: 原始数据流处理 ==========
#define RAW_BUFFER_SIZE 1000
unsigned char raw_buffer[RAW_BUFFER_SIZE];
int raw_index = 0;
#endif

void setup() {
  Serial.begin(115200);  // 调试用
  Serial.println("RPLiDAR Data Logger Starting...");
  
  Serial2.begin(baud2);
  Serial2.write(0xA5);
  Serial2.write(0x20);  
  
  pinMode(Lidar_ctr, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(LEFT_MOTO,OUTPUT);
  pinMode(RIGHT_MOTO,OUTPUT); 

  analogWrite(Lidar_ctr, 255);  
  bluetooth.begin(baud);  
  SD_module_init(CS);     
  
  // 创建对应的文件
  char filename[50];
  sprintf(filename, "Database/solution%d.txt", SOLUTION);
  if(!SD.exists(filename)) {
    f = SD.open(filename, FILE_WRITE);
    if(f) {
      f.close();
    }
  }
  
  Serial.println("Setup complete");
}

void loop() {
#if SOLUTION == 1
  solution1_strict_header();
#elif SOLUTION == 2
  solution2_timestamp();
#elif SOLUTION == 3
  solution3_packet_count();
#elif SOLUTION == 4
  solution4_raw_stream();
#endif
}

#if SOLUTION == 1
//========== 解决方案1: 严格包头验证 ==========
void solution1_strict_header() {
  while(Serial2.available()) {
    unsigned char incoming_byte = Serial2.read();
    
    if(!header_verified) {
      // 检查包头
      if(incoming_byte == expected_header[header_index]) {
        header_index++;
        if(header_index >= 7) {
          header_verified = true;
          header_index = 0;
          packet_byte_count = 0;
          Serial.println("Header verified, reading data...");
        }
      } else {
        header_index = 0;  // 重置包头检测
      }
    } else {
      // 读取数据包
      data_packet[packet_byte_count] = incoming_byte;
      packet_byte_count++;
      
      if(packet_byte_count >= 5) {
        // 处理完整的5字节数据包
        processDataPacket_solution1();
        packet_byte_count = 0;
      }
    }
  }
  
  // 写入完成的圈数据
  if(round_complete) {
    writeRoundData_solution1();
    round_complete = false;
  }
}

void processDataPacket_solution1() {
  // 添加到圆圈缓冲区
  for(int i = 0; i < 5; i++) {
    round_buffer[round_buffer_index++] = data_packet[i];
  }
  
  // 检查是否是新圆圈开始
  if((data_packet[0] & 0x03) == 0x01 && round_buffer_index > 5) {
    round_complete = true;
  }
  
  // 防止缓冲区溢出
  if(round_buffer_index >= sizeof(round_buffer) - 10) {
    round_complete = true;
  }
}

void writeRoundData_solution1() {
  f = SD.open("Database/solution1.txt", FILE_WRITE);
  if(f) {
    f.print("ROUND_START,");
    for(int i = 0; i < round_buffer_index; i++) {
      f.print(round_buffer[i]);
      f.print(",");
    }
    f.println("ROUND_END");
    f.close();
    
    Serial.print("Round written: ");
    Serial.print(round_buffer_index);
    Serial.println(" bytes");
  }
  
  round_buffer_index = 0;
}

#elif SOLUTION == 2
//========== 解决方案2: 基于时间戳的换行 ==========
void solution2_timestamp() {
  unsigned long current_time = millis();
  
  while(Serial2.available()) {
    unsigned char incoming_byte = Serial2.read();
    
    // 添加时间戳标记（每200ms一次，对应RPLiDAR一圈）
    if(current_time - last_round_time >= ROUND_TIMEOUT_MS) {
      if(total_bytes_in_round > 0) {
        // 写入时间戳和换行
        buf_index += sprintf(&BUFFER[buf_index], "\nTIME:%lu,BYTES:%d\n", 
                           current_time, total_bytes_in_round);
        total_bytes_in_round = 0;
      }
      last_round_time = current_time;
    }
    
    // 存储数据
    buf_index += sprintf(&BUFFER[buf_index], "%d,", incoming_byte);
    total_bytes_in_round++;
    
    // 写入SD卡
    if(buf_index >= bufsize - 100) {
      f = SD.open("Database/solution2.txt", FILE_WRITE);
      if(f) {
        f.write(BUFFER, buf_index);
        f.close();
      }
      buf_index = 0;
    }
  }
}

#elif SOLUTION == 3
//========== 解决方案3: 基于数据包计数 ==========
void solution3_packet_count() {
  while(Serial2.available()) {
    unsigned char incoming_byte = Serial2.read();
    
    temp_buffer[packet_byte_count] = incoming_byte;
    packet_byte_count++;
    
    if(packet_byte_count >= 5) {
      // 存储5字节数据包
      buf_index += sprintf(&BUFFER[buf_index], "P%d:", packet_count);
      for(int i = 0; i < 5; i++) {
        buf_index += sprintf(&BUFFER[buf_index], "%d,", temp_buffer[i]);
      }
      
      packet_count++;
      packet_byte_count = 0;
      
      // 每PACKETS_PER_ROUND个包换行
      if(packet_count >= PACKETS_PER_ROUND) {
        buf_index += sprintf(&BUFFER[buf_index], "\nROUND_COMPLETE\n");
        packet_count = 0;
      }
      
      // 写入SD卡
      if(buf_index >= bufsize - 200) {
        f = SD.open("Database/solution3.txt", FILE_WRITE);
        if(f) {
          f.write(BUFFER, buf_index);
          f.close();
        }
        buf_index = 0;
      }
    }
  }
}

#elif SOLUTION == 4
//========== 解决方案4: 原始数据流 ==========
void solution4_raw_stream() {
  while(Serial2.available() && raw_index < RAW_BUFFER_SIZE - 1) {
    raw_buffer[raw_index] = Serial2.read();
    raw_index++;
  }
  
  // 当缓冲区满时，处理并写入
  if(raw_index >= RAW_BUFFER_SIZE - 1) {
    processRawData();
    raw_index = 0;
  }
}

void processRawData() {
  f = SD.open("Database/solution4.txt", FILE_WRITE);
  if(f) {
    f.print("RAW_BLOCK_START:");
    f.print(millis());
    f.print(",");
    
    for(int i = 0; i < RAW_BUFFER_SIZE; i++) {
      f.print(raw_buffer[i]);
      f.print(",");
      
      // 每50字节换行（便于分析）
      if((i + 1) % 50 == 0) {
        f.println();
      }
    }
    
    f.println("RAW_BLOCK_END");
    f.close();
    
    Serial.print("Raw block written: ");
    Serial.println(RAW_BUFFER_SIZE);
  }
}
#endif

// 通用函数
void handleBluetooth() {
  if(bluetooth.available()) {
    receive_bt = bluetooth.read();
    slide_control(receive_bt, V_car, LEFT_MOTO, RIGHT_MOTO, Sensi);
    if(receive_bt == 200) {
      Serial2.write(0xA5);
      Serial2.write(0x25);
    }
  }
}