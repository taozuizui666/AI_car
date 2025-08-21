#include "control.h"

void ahead(int left_moto,int right_moto)
{
    analogWrite(left_moto, 200);
    analogWrite(right_moto, 200);
}

void left(int left_moto,int right_moto)
{
    analogWrite(left_moto, 0);
    analogWrite(right_moto, 200);
}

void right(int left_moto,int right_moto)
{
    analogWrite(left_moto, 200);
    analogWrite(right_moto, 0);
}

void left_ahead(int left_moto,int right_moto)
{
    analogWrite(left_moto, 200);
    analogWrite(right_moto, 255);
}

void right_ahead(int left_moto,int right_moto)
{
    analogWrite(left_moto, 255);
    analogWrite(right_moto, 200);
}

void stop(int left_moto,int right_moto)
{
    analogWrite(left_moto, 0);
    analogWrite(right_moto, 0);    
}