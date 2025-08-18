#ifndef CONTROL
#define CONTROL
// #include "WString.h"
#include <Arduino.h>

void ahead(int left_moto,int right_moto);
void left(int left_moto,int right_moto);
void right(int left_moto,int right_moto);
void left_ahead(int left_moto,int right_moto);
void right_ahead(int left_moto,int right_moto);
void stop(int left_moto,int right_moto);

#endif