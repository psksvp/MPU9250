#ifndef __MPU9250_LIB__
#define __MPU9250_LIB__

struct Orientation
{
  double roll;
  double pitch;
  double yaw;
};

typedef struct Orientation Orientation;

struct MPU9250Data
{ 
  int valid; 
  
  double humidity;
  double pressure;
  double height;      // meter
  double temperature; // c
  
  // degree
  Orientation pose;
  Orientation gyro;
  Orientation accel;
  Orientation compass;
};

typedef struct MPU9250Data MPU9250Data;

#ifdef __cplusplus
extern "C" {
#endif
  
int MPU9250Start();
void MPU9250Stop();
MPU9250Data MPU9250Poll();

#ifdef __cplusplus
}
#endif

#endif