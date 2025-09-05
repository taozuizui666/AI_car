import serial
from matplotlib import pyplot as plt

Lidar_data_bin_num = 5
Lidar_head_num = 7
ser = None

while not ser:
    try:
        ser = serial.Serial('COM12', 115200, timeout=1)
    except serial.SerialException:
        continue

data = ser.read(Lidar_head_num)
head_values = ' '.join(f'{b:02X}' for b in data)
if head_values =="A5 5A 05 00 00 40 81":
    print("yes")

    # plt.ion()  # 打开交互模式
    # fig, ax = plt.subplots(subplot_kw={'projection': 'polar'})
    # ax.set_theta_zero_location('N')  # 设置0度方向为北（向上）
    # ax.set_ylim(0, 10)  # 距离范围，根据实际情况调整
    # line, = ax.plot([], [], 'ro')  # 初始化线对象
    # background = fig.canvas.copy_from_bbox(ax.bbox)  # 保存背景

    while True:
        data = ser.read(Lidar_data_bin_num)
        if len(data) == Lidar_data_bin_num:  
            first_byte = data[0]
            angle_raw = ((data[2]<<8) | data[1]) >>1
            distance_raw = (data[4]<<8) | data[3]
            angle_real = angle_raw/64.
            distance_real = distance_raw/4000.
            print(f"Angle: {angle_real:.2f} deg, Distance: {distance_real:.3f} m\n")

            # # 更新数据
            # line.set_xdata([angle_real * 3.1415926 / 180])  # 将角度转换为弧度
            # line.set_ydata([distance_real])

            # # 使用 blit 技术加速绘图
            # fig.canvas.restore_region(background)  # 恢复背景
            # ax.draw_artist(line)  # 重绘线条
            # fig.canvas.blit(ax.bbox)  # 更新变动部分