# **AI\_car**

*a self-driving car project based on AI model*

author ： TAO RAN



### **note**：

when compilation wrong in ardunio, try remove C\\...\\AppData\\Local\\arduino\\sketches files, this may make recompile successful.

the whole program is divided into 3 main parts : 
* **part1**: build the car and run program (bluetooth control, Lidar scan and SD store )
* **part2**: train the car through App on the phone
* **part3**: use ML methods to train a model to enable the car self-control

## 


### **program code address** :
teensy_project\C_code\AIcar\main_lidar_SD_test -now yield really now\main
### **App address** :
teensy_project\Androidapp\App

##

record : after 3 months, the car can use store lidar's data into SD now.

it took me less than 1 month to complete 90% of Part1, but it took me more than 2 months to complete the last 10%.

It is very strange that when I use computer to receive data from Lidar, it is all ok, when I store data into SD card, it is all ok too no matter how many data I stored one time( you can see the codes in the C_code folder), but when I combined them together, there are strange errors all the time,  like: reset the chip after 5~10s, file remains empty, can't control the car after 5~10s ...

I think SD's base communication -- SPI is too slow, so when storing data into SD, it block the CPU from getting and processing with Lidar data, but Lidar data is very huge and fast, this could make the data buffer(the place to store received data from Lidar) collapse after few rounds of storing to SD.

I should use DMA to directly store data into SD, this can make CPU easy to work on Lidar's data processing and bluetooth. But I failed to configure DMA on teensy4.0, I think you can solve the previous problem by using DMA method.

now I am using "yield()" function to handle this, "yield()" can stop the current process, check USB message and reset the watch_dog timer. it is said in these way can handle the chip reset problem and it dose, but I still don't know whether it is the true reason, later I will do other experiences to research this.
```
        for(int i=0; i<num_of_round; i++){
          f.print(Lidar_list_dis[i]);
          f.print(',');
          // every storing 50 data, reset the watch_dog timer to tell the system is all OK
          if (i % 50 == 0) yield(); 
        }

```

another important method I use is to clean the Lidar_data buffer after storing data to SD, to prevent the buffer collapse after been blocked for so long .

```
      //-----codes that store data to SD-----

      while(Serial2.available()) {
        Serial2.read(); 
      }

```
This make me learn a lesson: we shouldn't just go after the speed, we should also take stability into consideration. Dr. Gerard told me sometimes it's very useful to make the fast process wait sometime for the slow process.