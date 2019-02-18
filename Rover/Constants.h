#ifndef BoR_1819_CONSTANTS_GUARD_H
#define BoR_1819_CONSTANTS_GUARD_H
#include <ArduCAM.h>
#include <MPU9250.h>

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
#define PROTOCOL_BAUD 9600

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

// Incoming Messages
enum ActionType {
  TAKE_PHOTO,
  GET_STAT,
  SOIL_SAMPLE,
  PING
};

// Outgoing Messages
enum OutgoingMessageType {
  ERR,
  PIC,
  STATUS,
  ROUTINE_COMPLETE,
  PONG
};

enum OutgoingMessageStatus {
  M_OK=200,
  M_ERR=500,
  M_BUSY=503,
  M_UNPARSEABLE=400
};
//---------------------------//

//--------Positioning--------//
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME280_I2C_Wire Wire
#define BME280_I2C_ADDR 0x76
#define BME280_QUALITY_ARGUMENTS Adafruit_BME280::MODE_NORMAL, Adafruit_BME280::SAMPLING_X2, /* temperature */ Adafruit_BME280::SAMPLING_X16, /* pressure */ Adafruit_BME280::SAMPLING_X1, /* humidity */ Adafruit_BME280::FILTER_X16, Adafruit_BME280::STANDBY_MS_0_5
#define MPU9250_I2C_Wire Wire
#define MPU9250_I2C_Clock 400000
#define MPU9250_I2C_Address MPU9250_ADDRESS_AD0
#define MPU9250_I2C_CORRECT_WHOAMI 0x73
#define MAGNETIC_DECLINATION 14.3
//---------------------------//

//-----Landing Detection-----//
#define LD_APOGEE_MINIMUM 4        // Meters
#define LD_GROUND_TRIG_DEADBAND 1    // Meters
#define LD_GROUND_DELAY 10           // Seconds (Time-To-Wait for steadiness after landing)
#define LD_GROUND_STEADY_DEADBAND 1  // Meters

#define LD_EWMA_SMOOTHING_ALPHA 0.1 // between 0 and 1

#define LD_TIME_FALLBACK_PRE_TAKEOFF 0   // seconds, assume that it takes off this many seconds in the future if the altimiter breaks
#define LD_TIME_FALLBACK_APOGEE 30      // seconds, assume that it reaches apogee after takeoff at a maximum of this many seconds later
#define LD_TIME_FALLBACK_DESCEND 60     // seconds, assume that it lands after apogee at a maximum of this many seconds later
#define LD_TIME_FALLBACK_GROUND_DELAY 20 // seconds, assume that it takes this long to settle after falling within the allowable ground deadband
//---------------------------//

//----------Camera-----------//
const int cam_resolution = OV2640_640x480; //OV2640_1600x1200;
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
