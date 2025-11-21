#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "control_slide.h"
// ports
#define NUM 50000000
#define CS          10  //chip select(from SD_card module)
File f;

float x=0.0;
void setup() {  
  Serial.begin(115200);
  Serial.println("bad");
  SD.begin(CS);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  delay(2000);
  digitalWrite(2, LOW);
}
void loop() {

  int i=0;
  f = SD.open("file0.txt",FILE_WRITE);
  for(i=0;i<NUM;i++){
    x+=2;
    f.print(x);
  }
  f.close();

  Serial.print("end one round");
  //--------------end of loop()-------------
}