#include "BT_control.h"

void bluetooth_control(int receive_data,int left_moto,int right_moto)
{
    switch (receive_data) {
    case 1:
      left_ahead(left_moto,right_moto);
      break;
    case 2:
      ahead(left_moto,right_moto);
      break;
    case 3:
      right_ahead(left_moto,right_moto);
      break;
    case 4:
      left(left_moto,right_moto);
      break;
    case 5:
      right(left_moto,right_moto);
      break;
    case 0:
      stop(left_moto,right_moto);
      break;
    default:
      break;
    }
}