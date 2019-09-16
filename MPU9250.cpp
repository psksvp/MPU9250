/**
 PiSensors JVM jni module
 jni interface for Raspberry pi sense hat's IMU sensors
 
 The BSD 3-Clause License
 Copyright (c) 2015, Pongsak Suvanpong (psksvp@gmail.com)
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 3. Neither the name of the copyright holder nor the names of its contributors may
 be used to endorse or promote products derived from this software without
 specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

#include <RTIMULib.h>
#include <cstdio>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "MPU9250.h"

RTIMUSettings* mySetting = NULL;
RTHumidity* myHumidity = NULL;
RTPressure* myPressure = NULL;
RTIMU* myIMU = NULL;

using namespace std;

void MPU9250Stop()
{
  if(NULL != mySetting)
    delete mySetting;
  if(NULL != myHumidity)
    delete myHumidity;
  if(NULL != myPressure)
    delete myPressure;
  if(NULL != myIMU)
    delete myIMU;
}

int MPU9250Start()
{
  if(NULL == mySetting)
  {
    const char *homedir = getenv("HOME");

    if ((homedir = getenv("HOME")) == NULL)
    {
      homedir = getpwuid(getuid())->pw_dir;
    }

    if(NULL == homedir)
    {
      std::fprintf(stderr, "error getting home path while loading RTIMULib.ini");
      return 0;
    }

    std::string strHomePath = homedir;
    std::string strINIPath = strHomePath + "/.config/RTIMULib.ini";


    mySetting = new RTIMUSettings(strINIPath.c_str());
    
    myIMU = RTIMU::createIMU(mySetting);
    if((myIMU == NULL) || (myIMU->IMUType() == RTIMU_TYPE_NULL))
    {
      std::fprintf(stderr, "IMUStart(), no IMU found");
      if(NULL != myIMU)
      {
        delete myIMU;
        myIMU = NULL;
      }
      return 0;
    }
    else
    {
      myIMU->IMUInit();
      myIMU->setSlerpPower(0.02);
      myIMU->setGyroEnable(true);
      myIMU->setAccelEnable(true);
      myIMU->setCompassEnable(true);
    }
    
    myHumidity = RTHumidity::createHumidity(mySetting);
    if(false == myHumidity->humidityInit())
    {
      std::fprintf(stderr, "IMUStart(), Fail to init Humidity sensor");
      delete myHumidity;
      myHumidity = NULL;
      return 0;
    }
    myPressure = RTPressure::createPressure(mySetting);
    if(false == myPressure->pressureInit())
    {
      std::fprintf(stderr, "IMUStart(), Fail to init Pressure sensor");
      delete myPressure;
      myPressure = NULL;
      return 0;
    }
  }
  return 1;
}

MPU9250Data MPU9250Poll()
{
  usleep(myIMU->IMUGetPollInterval() * 1000);
  
  MPU9250Data env;
  env.valid = 1;
  if(myIMU->IMURead())
  {
    RTIMU_DATA data = myIMU->getIMUData();
    
    env.pose.roll = data.fusionPose.x()  ;
    env.pose.pitch = data.fusionPose.y() ;
    env.pose.yaw = data.fusionPose.z()   ;
    
    env.gyro.roll = data.gyro.x() ;
    env.gyro.pitch = data.gyro.y();
    env.gyro.yaw = data.gyro.z()  ;
    
    env.accel.roll = data.accel.x() ;
    env.accel.pitch = data.accel.y();
    env.accel.yaw = data.accel.z()  ;
    
    env.compass.roll = data.compass.x()  ;
    env.compass.pitch = data.compass.y() ;
    env.compass.yaw = data.compass.z()   ;
  
  
    if(NULL != myHumidity)
    {
      myHumidity->humidityRead(data);
      env.humidity = data.humidity;
    }
    else
    {
      std::fprintf(stderr, "humidity is NULL");
      env.humidity = 0.0;
    }
  
    if(NULL != myPressure)
    {
      myPressure->pressureRead(data);
      env.pressure = data.humidity;
      env.temperature = data.temperature;
      env.height = RTMath::convertPressureToHeight(data.pressure);
    }
    else
    {
      std::fprintf(stderr, "pressure is NULL");
      env.pressure = 0.0;
      env.temperature = 0.0;
    }
  }
  else
  {
    std::fprintf(stderr, "IMUPoll sensors read fail");
    env.valid = 0;
  }
  return env;
}

