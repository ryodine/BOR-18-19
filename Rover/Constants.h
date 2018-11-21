#ifndef BoR_1819_CONSTANTS_GUARD_H
#define BoR_1819_CONSTANTS_GUARD_H
#include <ArduCAM.h>

/**
 * Constants for Arduino subsystems
 * 
 * @author Ryan D. Johnson
 * 
 * WPI Battle of the Rockets 2018-2019
 * Team rocket Powered Locomotive
 */
 
//----------HARDWARE----------//
#define ROATION_TRANSLATION_RATIO
//---------------------------//


//----------PINOUTS----------//
#define PIN_CAM_SPI_CS 10

//---------------------------//


//------PID CONTROLLER-------//
#define ERROR_BAND 1000
#define RPM_AVG_MILLIS 10
//---------------------------//

//-------Communication-------//
#define DEBUG_BAUD 9600
#define PROTOCOL_BAUD 115200

enum CommReturnCode {
  /** OK Codes **/
  OK,
  NO_MESSAGE,
  MESSAGE_AWAITS,
  SENTINEL_END_OK,

  /** Bad Codes **/
  DOUBLE_INITIALIZE,
  UNINITIALIZED,
  SENTINEL_END_ERROR
};

enum ActionType {
  TAKE_PHOTO
};

enum OutgoingMessageType {
  ERR,
  PIC
};

enum OutgoingMessageStatus {
  M_OK=200,
  M_ERR=500
};
//---------------------------//

//----------Camera-----------//
const int cam_resolution = OV2640_1600x1200;
//---------------------------//

//-----PID COEFFICIENTS------//
struct PIDConstant {
  double Kf;
  double Kp;
  double Ki;
  double Kd;
};
                                    /*Kf    Kp     Ki   Kd*/
//const PIDConstant rightwheel = {    0,    4.5,  0.04,   0};
//const PIDConstant leftwheel =  {    0,    4.5,  0.04,   0};
//---------------------------//

#endif
