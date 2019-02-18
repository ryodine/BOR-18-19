#ifndef BoR_1819_PHOTOGRAPHY_LAYER_GUARD_H
#define BoR_1819_PHOTOGRAPHY_LAYER_GUARD_H

/**
 * Take photos, and get the bytes.
 * 
 * @author Ryan D. Johnson
 * 
 * WPI Battle of the Rockets 2018-2019
 * Team rocket Powered Locomotive
 */

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <Stream.h>
#include "memorysaver.h"
#include "Constants.h"
#include "DebugDisplay.h"

class RoverCamera {
  public:
    RoverCamera(unsigned int cs, Stream* debugOutput, DebugDisplay* debugdisplay) : myCAM(OV2640, cs), debugStream(debugOutput), debugDisplay(debugdisplay) {};
    void begin();
    void capture(void (*szcallback)(unsigned int len), void (*bytecallback)(byte* bytes, unsigned int len));
    void reset();
    
  private:
    ArduCAM myCAM;
    Stream* debugStream;
    DebugDisplay* debugDisplay;
  
};

#endif
