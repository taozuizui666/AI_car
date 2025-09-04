#ifndef LIDAR_READ
#define LIDAR_READ

class Angle{
    unsigned char C_ang_l;
    unsigned char ang_h;
};

class Dist{
    unsigned char dist_l;
    unsigned char dist_h;
};

class Lidar_data{
    unsigned char S_quality;
    Angle   angle;
    Dist    dist;
};

#endif