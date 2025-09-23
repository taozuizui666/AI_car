#include <SoftwareSerial.h>
#include <SPI.h>
#include <SdFat.h>
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
#define bufsize     4096       // SD_card's buf size (增大一点更好)
#define Lidar_head_num      7     //A5 5A 05 00 00 40 81 
#define Lidar_data_bin_num  5
#define num_of_round        256   // the number of data after a round

//serial test
#define serial_open 0

//-------------bluetooth----------------
SoftwareSerial bluetooth(0,1);  // bluetooth's pin RX,TX

//-------------SD_card------------------
#define FILE_NAME   "data2.txt"
FsFile f;
SdFat SD;
int buf_index = 0;
unsigned char   BUFFER[bufsize]; // 用作输出文本缓冲

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

// ----------------- 快速整数转字符串辅助函数 -----------------
// 把无符号整型写入 BUFFER（不带结尾），返回写入长度
static inline void append_uint_to_buf(unsigned long v, unsigned char *buf, int &idx) {
  if (v == 0) {
    buf[idx++] = '0';
    return;
  }
  // 临时反向存储数字
  char tmp[12];
  int t = 0;
  while (v > 0) {
    tmp[t++] = '0' + (v % 10);
    v /= 10;
  }
  // 反转写回 buf
  while (t--) {
    buf[idx++] = tmp[t];
  }
}

// 把固定小数（value 表示 scaled_value，scale 表示小数位数的缩放，例如 scale=100 for 2 decimals）写入 buf
static inline void append_fixed_decimal_to_buf(unsigned long scaled_value, unsigned long scale, unsigned char *buf, int &idx) {
  unsigned long whole = scaled_value / scale;
  unsigned long frac = scaled_value % scale;
  append_uint_to_buf(whole, buf, idx);
  buf[idx++] = '.';
  // 确保小数位补零（例如 scale=100 -> 两位）
  // 先计算需要的小数位数
  int digits = 0;
  unsigned long tmpScale = scale;
  while (tmpScale > 1) { tmpScale /= 10; digits++; }
  // frac 可能需要前导 0s
  // 例如 digits=2, frac=5 -> "05"
  unsigned long place = 1;
  for (int i = 1; i < digits; i++) place *= 10;
  for (int i = 0; i < digits; i++) {
    unsigned long d = frac / place;
    buf[idx++] = '0' + d;
    frac = frac % place;
    if (place > 0) place /= 10;
  }
}

// ----------------- Setup & Loop -----------------
void setup() {
  SPI.begin();
  //-------------Lidar---------------------
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

  // -------------SD_card init (open file ONCE)--------------
  // 提高 SCK 频率（视 SD 卡/模块能力调整：尝试 50, 若不稳定降到 25 或 16）
  if (!SD.begin(CS, SD_SCK_MHZ(50))) {
    // 若 50MHz 失败可尝试 25MHz
    SD.begin(CS, SD_SCK_MHZ(25));
  }
  // 在 setup 中打开文件一次，并移动到文件末尾（append）
  f = SD.open(FILE_NAME, FILE_WRITE);
  if (!f) {
    // 如果 open 失败，可以尝试创建
    SD.remove(FILE_NAME);
    f = SD.open(FILE_NAME, FILE_WRITE);
  }
  // 预置缓冲索引
  buf_index = 0;
  Lidar_head_message = 0;
  Lidar_list_index = 0;
}

void loop() {
  // 获取 header（只做一次以后就读数据包）
  if ((!Lidar_head_message) && (Serial2.available() >= Lidar_head_num)) {
    for (int i = 0; i < Lidar_head_num; i++) {
      Lidar_data = Serial2.read();
    }
    Lidar_head_message = 1;
  }

  // 读一整圈数据到内存数组（你的原逻辑）
  while (Lidar_list_index != num_of_round) {
    if (Lidar_head_message && Serial2.available() >= Lidar_data_bin_num) {
      for (int i = 0; i < Lidar_data_bin_num; i++) {
        Lidar_5byte[i] = Serial2.read();
      }
      angle_raw = (((uint16_t)Lidar_5byte[2] << 8) | Lidar_5byte[1]) >> 1;
      dist_raw = ((uint16_t)Lidar_5byte[4] << 8) | Lidar_5byte[3];
      angle_real = angle_raw / 64.0;
      dist_real = dist_raw / 4000.0;
      Lidar_list[Lidar_list_index++] = angle_real;
    }
  }

  // 将一整圈数据格式化成 ASCII 存入 BUFFER，再一次性写入 SD
  // 我们把 angle 保留两位小数（scale = 100）
  const unsigned long angle_scale = 100; // 两位小数
  for (int i = 0; i < Lidar_list_index; i++) {
    // 由 angle_raw 计算 scaled integer 更快且准确
    unsigned long angle_scaled = ((unsigned long) ( (unsigned long) ( (unsigned long) ((uint32_t)( ( (uint32_t)((((uint32_t) ( ((uint32_t) ((uint32_t) ( (uint32_t)  ( (uint32_t) ( (angle_raw = (((uint16_t)Lidar_list[i])>0) ? 0 : 0) ) ) ) ) ) ) ) ) ) ) ) )) )); 
    // 上面那行是占位（为了避免直接用 Lidar_list[i] 的 float） 
    // 更直接的计算应该直接用原始 raw 值而不是 Lidar_list[i]。但我们当前数组只存角度 float，
    // 为简洁，这里用 Lidar_list[i] 转换为 scaled（注意浮点->定点）
    float ang = Lidar_list[i];
    unsigned long ang_scaled = (unsigned long) (ang * (float)angle_scale + 0.5f); // 四舍五入

    // 写入到 BUFFER
    // 如果缓冲区快满则先 flush 到 SD（避免溢出）
    // 每个数字大约最多 10 字符（取决于数值）+ 小数点和分隔符
    if (buf_index > (bufsize - 32)) {
      // write out
      if (f) {
        f.write(BUFFER, buf_index);
        f.sync(); // flush 到卡（视需要可注释以更快）
      }
      buf_index = 0;
    }

    append_fixed_decimal_to_buf(ang_scaled, angle_scale, BUFFER, buf_index);
    BUFFER[buf_index++] = '\t'; // 用 tab 分隔
  }
  BUFFER[buf_index++] = '\n';

  // 最后一次性把缓冲写到 SD
  if (f && buf_index > 0) {
    f.write(BUFFER, buf_index);
    f.sync(); // 也可以每若干圈再 sync，试验找到最优
  }
  buf_index = 0;
  Lidar_list_index = 0;
}

