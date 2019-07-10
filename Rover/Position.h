#ifndef BoR_1819_POSITION_SENSING_GUARD_H
#define BoR_1819_POSITION_SENSING_GUARD_H

/**
 * 9DOF and Altimeter control (10DOF)
 * 
 * @author Ryan D. Johnson
 * 
 * WPI Battle of the Rockets 2018-2019
 * Team rocket Powered Locomotive
 * 
 * The rover knows where it is at all times. It knows this because it knows where it isn't. 
 * By subtracting where it is from where it isn't, or where it isn't from where it is 
 * (whichever is greater), it obtains a difference, or deviation. The guidance subsystem uses 
 * deviations to generate corrective commands to drive the rover from a position where it 
 * is to a position where it isn't, and arriving at a position where it wasn't, it now is. 
 * Consequently, the position where it is, is now the position that it wasn't, and it follows 
 * that the position that it was, is now the position that it isn't. 
 * 
 * In the event that the position that it is in is not the position that it wasn't, the system
 * has acquired a variation, the variation being the difference between where the rover is,
 * and where it wasn't. If variation is considered to be a significant factor, it too may be
 * corrected by the GEA. However, the rover must also know where it was.
 * 
 * The rover guidance computer scenario works as follows. Because a variation has modified
 * some of the information the rover has obtained, it is not sure just where it is. However,
 * it is sure where it isn't, within reason, and it knows where it was. It now subtracts where
 * it should be from where it wasn't, or vice-versa, and by differentiating this from the
 * algebraic sum of where it shouldn't be, and where it was, it is able to obtain the deviation
 * and its variation, which is called error.
 */

#include <quaternionFilters.h>
#include <MPU9250.h>
#include <Stream.h>
#include <PrintEx.h>
#include <Adafruit_BME280.h>
#include "Constants.h"
#include "DebugDisplay.h"

#define ALTIMITER_ERROR_MASK 0b00000001
#define ALTIMITER_FAIL_MASK 0b00000010
#define MPU9250_FAIL_MASK 0b00000100
#define MAGNETO_FAIL_MASK 0b00001000

class PositionSensing {
  public:
    PositionSensing(Stream* debugOutput, DebugDisplay* debugdisplay) : debugStream(debugOutput), debugDisplay(debugdisplay), debug(false), AHRS_mode(true), calibrate_magneto(false), zYaw(0.0f), zPitch(0.0f), zRoll(0.0f) {};
    void tick();
    void begin();
    void print();
    void zero();
    float getPitch();
    float getYaw();
    float getRoll();
    float getAltitude();
    float getPressure();
    float getTemperature();
    float getHumidity();
    byte getErrorFlags() { return errorflags; };
    bool hasError() { return errorflags != 0; };
  private:
    float zYaw, zPitch, zRoll;
    MPU9250* imu;
    Adafruit_BME280* alt;
    Stream* debugStream;
    DebugDisplay* debugDisplay;
    boolean debug;
    boolean AHRS_mode;
    boolean calibrate_magneto;

    
    float baseline_alt; // Should be backed up
    float pressure;
    float temperature;
    float humidity;
    float altitude;

    const int altimiter_retries = 15;
    int altimiter_retries_remaining = altimiter_retries;
                      
    byte errorflags = 0b00000000;
                     /* |||||||+-- Altimiter Broke
                      * ||||||+-- Altimiter failed to initialize at boot
                      * |||||+-- MPU9250 Failed at start
                      * ||||+-- Magneto fail
                      * |||+--
                      * ||+--
                      * |+--
                      * +--
                      */
};

 #endif
