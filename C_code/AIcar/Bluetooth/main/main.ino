/*
 * Teensy 4.0 配置 HC-05 蓝牙模块
 * 
 * 连接方式：
 * HC-05 VCC  -> Teensy 5V (或 3.3V，取决于模块)
 * HC-05 GND  -> Teensy GND
 * HC-05 TXD  -> Teensy RX1 (Pin 0)
 * HC-05 RXD  -> Teensy TX1 (Pin 1) - 注意：如果HC-05是3.3V需要加分压电路
 * HC-05 EN/KEY -> Teensy Pin 2 (用于进入AT模式)
 * 
 * 使用说明：
 * 1. 上传代码前，不要连接HC-05的VCC
 * 2. 上传代码后，将HC-05的KEY/EN引脚连接到Pin 2并保持高电平
 * 3. 然后给HC-05上电（连接VCC）
 * 4. HC-05会以38400波特率进入AT模式（LED慢闪）
 * 5. 打开串口监视器（波特率9600）
 * 6. 输入AT命令进行配置
 */

// 定义串口
#define HC05_Serial Serial1  // 使用硬件串口1
#define DEBUG_Serial Serial  // USB串口用于调试

// HC-05 EN/KEY引脚
#define HC05_EN_PIN 2

void setup() {
  // 初始化USB串口（用于电脑通信）
  DEBUG_Serial.begin(9600);
  while (!DEBUG_Serial && millis() < 3000); // 等待串口打开，最多3秒
  
  // 设置EN引脚为高电平，使HC-05进入AT模式
  pinMode(HC05_EN_PIN, OUTPUT);
  digitalWrite(HC05_EN_PIN, HIGH);
  
  // 初始化与HC-05通信的串口
  // HC-05在AT模式下默认波特率是38400
  HC05_Serial.begin(38400);
  
  delay(1000);
  
  DEBUG_Serial.println("========================================");
  DEBUG_Serial.println("HC-05 蓝牙模块配置工具");
  DEBUG_Serial.println("========================================");
  DEBUG_Serial.println("请确保HC-05已进入AT模式（LED慢闪）");
  DEBUG_Serial.println();
  DEBUG_Serial.println("常用AT命令：");
  DEBUG_Serial.println("AT - 测试连接");
  DEBUG_Serial.println("AT+NAME=新名称 - 设置蓝牙名称");
  DEBUG_Serial.println("AT+PSWD=1234 - 设置配对密码（4位数字）");
  DEBUG_Serial.println("AT+UART=9600,0,0 - 设置波特率为9600");
  DEBUG_Serial.println("AT+ROLE=0 - 设置为从机模式");
  DEBUG_Serial.println("AT+ADDR? - 查询蓝牙地址");
  DEBUG_Serial.println("AT+VERSION? - 查询版本信息");
  DEBUG_Serial.println("AT+STATE? - 查询当前状态");
  DEBUG_Serial.println();
  DEBUG_Serial.println("快捷配置命令：");
  DEBUG_Serial.println("1 - 快速配置（名称：MyTeensy，密码：1234）");
  DEBUG_Serial.println("========================================");
  DEBUG_Serial.println();
}

void loop() {
  // 从USB串口读取数据并发送到HC-05
  if (DEBUG_Serial.available()) {
    String input = DEBUG_Serial.readStringUntil('\n');
    input.trim();
    
    // 快捷配置选项
    if (input == "1") {
      quickConfig();
    } else {
      // 发送AT命令到HC-05
      HC05_Serial.print(input);
      HC05_Serial.print("\r\n");
      DEBUG_Serial.print("发送: ");
      DEBUG_Serial.println(input);
    }
  }
  
  // 从HC-05读取响应并显示到USB串口
  if (HC05_Serial.available()) {
    String response = "";
    while (HC05_Serial.available()) {
      char c = HC05_Serial.read();
      response += c;
      delay(2);
    }
    DEBUG_Serial.print("接收: ");
    DEBUG_Serial.println(response);
  }
}

// 快速配置函数
void quickConfig() {
  DEBUG_Serial.println("\n开始快速配置...");
  delay(500);
  
  // 测试连接
  DEBUG_Serial.println("\n1. 测试连接...");
  sendATCommand("AT");
  delay(500);
  
  // 设置名称
  DEBUG_Serial.println("\n2. 设置名称为 MyTeensy...");
  sendATCommand("AT+NAME=MyTeensy");
  delay(500);
  
  // 设置密码
  DEBUG_Serial.println("\n3. 设置密码为 1234...");
  sendATCommand("AT+PSWD=1234");
  delay(500);
  
  // 设置为从机模式
  DEBUG_Serial.println("\n4. 设置为从机模式...");
  sendATCommand("AT+ROLE=0");
  delay(500);
  
  // 设置波特率为9600
  DEBUG_Serial.println("\n5. 设置波特率为 9600...");
  sendATCommand("AT+UART=9600,0,0");
  delay(500);
  
  DEBUG_Serial.println("\n========================================");
  DEBUG_Serial.println("配置完成！");
  DEBUG_Serial.println("蓝牙名称：MyTeensy");
  DEBUG_Serial.println("配对密码：1234");
  DEBUG_Serial.println("波特率：9600");
  DEBUG_Serial.println("========================================");
  DEBUG_Serial.println("注意：下次使用时需要将HC-05的串口波特率改为9600");
  DEBUG_Serial.println("并将EN引脚断开（或接低电平）退出AT模式");
  DEBUG_Serial.println();
}

// 发送AT命令的辅助函数
void sendATCommand(String command) {
  HC05_Serial.print(command);
  HC05_Serial.print("\r\n");
  DEBUG_Serial.print("发送: ");
  DEBUG_Serial.println(command);
  
  // 等待响应
  delay(500);
  if (HC05_Serial.available()) {
    String response = "";
    while (HC05_Serial.available()) {
      char c = HC05_Serial.read();
      response += c;
      delay(2);
    }
    DEBUG_Serial.print("接收: ");
    DEBUG_Serial.println(response);
  }
}