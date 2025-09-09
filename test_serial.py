import serial
import math
import numpy as np
from matplotlib import pyplot as plt

Lidar_data_bin_num = 5
Lidar_head_num = 7
ser = None
data_buff_num = 1000
angle_buff = []
dist_buff = []

while not ser:
    try:
        ser = serial.Serial('COM12', 115200, timeout=1)
    except serial.SerialException:
        continue

data = ser.read(Lidar_head_num)
head_values = ' '.join(f'{b:02X}' for b in data)

if head_values =="A5 5A 05 00 00 40 81":
    print("yes")
    # plt.ion()
    # fig,ax = plt.subplots(subplot_kw={'projection':'polar'})
    # ax.set_theta_zero_location('N')  # 设置 0 度在上方 (北)
    # ax.set_theta_direction(-1)      # 设置角度顺时针增加
    # ax.set_ylim(0, 10)               # 设置距离显示范围 (米)，根据需要调整
    # ax.set_title("RPLidar 实时扫描图")
    # scat = ax.scatter([], [], c='b', s=5) # s 是点的大小

    while True:
        data = ser.read(Lidar_data_bin_num)
        if len(data) == Lidar_data_bin_num:
            first_byte = data[0]
            angle_raw = ((data[2]<<8) | data[1]) >>1
            distance_raw = (data[4]<<8) | data[3]
            angle_real = angle_raw/64.
            distance_real = distance_raw/4000.
            print(f"Angle: {angle_real:.2f} deg, Distance: {distance_real:.3f} m\n")
            
            # angle_rad = math.radians(angle_real)
            # angle_buff.append(angle_rad)
            # dist_buff.append(distance_real)
            # if len(angle_buff)>data_buff_num:
            #     angle_buff.pop(0)
            #     dist_buff.pop(0)
            # scat.set_offsets(np.c_[angle_buff, dist_buff])
            # fig.canvas.draw()
            # fig.canvas.flush_events()
