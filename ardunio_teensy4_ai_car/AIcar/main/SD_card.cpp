#include "SD.h"
#include "SD_card.h"
void SD_module_init(int CS_port)
{
    SD.begin(CS_port);
    if(!SD.exists("Database"))
    {
        SD.mkdir("Database");
    }
    // if(!SD.exists("Database/data.csv"))
    // {
    //     f = SD.open("Database/data.csv",FILE_WRITE);
    //     if(f){
    //     // f.println("ms,sensor");
    //     f.close();
    //     }
    // }
}