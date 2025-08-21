#include "control_slide.h"

void stop(int left_moto,int right_moto)
{
    analogWrite(left_moto, 0);
    analogWrite(right_moto, 0);    
}

void ahead(int v_car,int left_moto,int right_moto)
{
    analogWrite(left_moto, v_car);
    analogWrite(right_moto, v_car);
}

void slide_control(int receive_bt,int v_car,int left_moto,int right_moto,int sensi)
{
    switch(receive_bt){
        case 200:
            stop(left_moto,right_moto);
            break;
        case 201:
            ahead(v_car,left_moto,right_moto);
            break;
        default:
            analogWrite(left_moto, v_car-(50-receive_bt)*sensi);
            analogWrite(right_moto, v_car+(50-receive_bt)*sensi);
            break;
    }
}