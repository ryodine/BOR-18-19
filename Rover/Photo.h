#ifndef BoR_1819_PHOTOGRAPHY_LAYER_GUARD_H
#define BoR_1819_PHOTOGRAPHY_LAYER_GUARD_H
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <Stream.h>
#include "memorysaver.h"

class RoverCamera {
  public:
    RoverCamera(unsigned int cs, Stream* debugOutput) : myCAM(OV2640, cs), debugStream(debugOutput) {};
    void begin();
    void capture(void (*szcallback)(unsigned int len), void (*bytecallback)(byte* bytes, unsigned int len));
    
  private:
    ArduCAM myCAM;
    Stream* debugStream;
  
};

#endif
