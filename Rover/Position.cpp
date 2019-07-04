/**
 * 9DOF and Altimeter control (and Temperature,too) (11DOF?)
 * 
 * @author Ryan D. Johnson
 * 
 * WPI Battle of the Rockets 2018-2019
 * Team rocket Powered Locomotive
 **/

#include "Position.h"

void PositionSensing::begin() {
  imu = new MPU9250(MPU9250_I2C_Address, MPU9250_I2C_Wire, MPU9250_I2C_Clock);
  alt = new Adafruit_BME280();
  
  debugStream->println(F("[IMU] BME280 Init Begin"));
  if (! alt->begin(BME280_I2C_ADDR, &BME280_I2C_Wire)) {
    Serial.println("[IMU] BME280 INIT: Could not find BME280 sensor");
    //STOP CODE
    errorflags = errorflags | ALTIMITER_FAIL_MASK;
    //debugDisplay->setStopCode("ALT");
  } else {
    alt->setSampling( BME280_QUALITY_ARGUMENTS );
    alt->takeForcedMeasurement();
    baseline_alt = alt->readAltitude(SEALEVELPRESSURE_HPA);
  }
  
  debugStream->println(F("[IMU] BMP180 Init Success"));
  byte c = imu->readByte(MPU9250_I2C_Address, WHO_AM_I_MPU9250);
  if (debugStream != NULL && debug) {
    debugStream->print(F("[IMU] MPU9250 Init: I AM 0x"));
    debugStream->println(c, HEX);
    debugStream->print(F("[IMU] MPU9250 Init: I should be 0x"));
    debugStream->println(MPU9250_I2C_CORRECT_WHOAMI, HEX);
  }
  if (c == MPU9250_I2C_CORRECT_WHOAMI) {
    if (debugStream != NULL) {
      debugStream->println(F("[IMU] MPU9250 Init: IMU Online. Configuring."));
    }
    //Power-on self test (POST)
    imu->MPU9250SelfTest(imu->selfTest);
    if (debugStream != NULL && debug) {
      debugStream->print(F("[IMU] MPU9250 Init: x-axis self test: acceleration trim within : "));
      debugStream->print(imu->selfTest[0],1); debugStream->println("% of factory value");
      debugStream->print(F("[IMU] MPU9250 Init: y-axis self test: acceleration trim within : "));
      debugStream->print(imu->selfTest[1],1); debugStream->println("% of factory value");
      debugStream->print(F("[IMU] MPU9250 Init: z-axis self test: acceleration trim within : "));
      debugStream->print(imu->selfTest[2],1); debugStream->println("% of factory value");
      debugStream->print(F("[IMU] MPU9250 Init: x-axis self test: gyration trim within : "));
      debugStream->print(imu->selfTest[3],1); debugStream->println("% of factory value");
      debugStream->print(F("[IMU] MPU9250 Init: y-axis self test: gyration trim within : "));
      debugStream->print(imu->selfTest[4],1); debugStream->println("% of factory value");
      debugStream->print(F("[IMU] MPU9250 Init: z-axis self test: gyration trim within : "));
      debugStream->print(imu->selfTest[5],1); debugStream->println("% of factory value");
    }
    // Calibrate gyro and accelerometers, load biases in bias registers
    imu->calibrateMPU9250(imu->gyroBias, imu->accelBias);
    
    //Initiate
    if (debugStream != NULL && debug) {
      debugStream->println(F("[IMU] MPU9250 Init: Attempting to init"));
    }
    imu->initMPU9250();
    if (debugStream != NULL && debug) {
      debugStream->println(F("[IMU] MPU9250 Init: Inititalized for active data mode...."));
    }

    byte d = imu->readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    if (debugStream != NULL && debug) {
      debugStream->print(F("[IMU] MPU9250 Init: AK8963 Magnetometer Init "));
      debugStream->print(F("I AM 0x"));
      debugStream->print(d, HEX);
      debugStream->print(F(" I should be 0x"));
      debugStream->println(0x48, HEX);
    }

    if (d != 0x48)
    {
      // Communication failed, stop here
      if (debugStream != NULL) {
        debugStream->println(F("[IMU] MPU9250 Init: Communication to AK8963 Magnetometer failed, abort!"));
        debugStream->flush();
      }
      errorflags = errorflags | MAGNETO_FAIL_MASK;
    }

    if (!(MAGNETO_FAIL_MASK & errorflags)) {
      // Get magnetometer calibration from AK8963 ROM
      imu->initAK8963(imu->factoryMagCalibration);
      // Initialize device for active mode read of magnetometer
      if (debugStream != NULL && debug) {
        debugStream->println("[IMU] MPU9250 Init: AK8963 initialized for active data mode....");
      }
  
      if (debugStream != NULL && debug) {
        //  debugStream->println("Calibration values: ");
        debugStream->print("[IMU] MPU9250 Init: X-Axis factory sensitivity adjustment value ");
        debugStream->println(imu->factoryMagCalibration[0], 2);
        debugStream->print("[IMU] MPU9250 Init: Y-Axis factory sensitivity adjustment value ");
        debugStream->println(imu->factoryMagCalibration[1], 2);
        debugStream->print("[IMU] MPU9250 Init: Z-Axis factory sensitivity adjustment value ");
        debugStream->println(imu->factoryMagCalibration[2], 2);
      }
    }

    imu->getAres();
    imu->getGres();
    imu->getMres();
    if (!(MAGNETO_FAIL_MASK & errorflags) && calibrate_magneto) {
      imu->magCalMPU9250(imu->magBias, imu->magScale);
    }
    if (!(MAGNETO_FAIL_MASK & errorflags) && debugStream != NULL && debug) {
      //debugStream->println("[IMU] MPU9250 Init: Done with calibration of Biases ");
      debugStream->print(F("[IMU] MPU9250 Init: AK8963 mag biases (mG) "));
      debugStream->print(imu->magBias[0]);
      debugStream->print(F(", "));
      debugStream->print(imu->magBias[1]);
      debugStream->print(F(", "));
      debugStream->println(imu->magBias[2]);
  
      debugStream->print("[IMU] MPU9250 Init: AK8963 mag scale (mG) ");
      debugStream->print(imu->magScale[0]);
      debugStream->print(F(", "));
      debugStream->print(imu->magScale[1]);
      debugStream->print(F(", "));
      debugStream->println(imu->magScale[2]);

      debugStream->println("[IMU] MPU9250 Init: Magnetometer:");
      debugStream->print("[IMU] MPU9250 Init: X-Axis sensitivity adjustment value ");
      debugStream->println(imu->factoryMagCalibration[0], 2);
      debugStream->print("[IMU] MPU9250 Init: Y-Axis sensitivity adjustment value ");
      debugStream->println(imu->factoryMagCalibration[1], 2);
      debugStream->print("[IMU] MPU9250 Init: Z-Axis sensitivity adjustment value ");
      debugStream->println(imu->factoryMagCalibration[2], 2);
    }
    
  } else if (debugStream != NULL) {
    debugStream->println(F("[IMU] MPU9250 Init: /!\\ FATAL ERROR I AM OFFLINE!!"));
    //STOP CODE
    errorflags = errorflags | MPU9250_FAIL_MASK;
  }
  if (debugStream != NULL && !(MPU9250_FAIL_MASK & errorflags)) {
    debugStream->println(F("[IMU] MPU9250 Init: Completed"));
  }
}

void PositionSensing::tick() {
  //Altimiter stuff first
  
  if (!(errorflags & ALTIMITER_FAIL_MASK)) {
    temperature = alt->readTemperature();
    pressure = alt->readPressure();
    float altitude_temp = altitude;
    altitude = alt->readAltitude(SEALEVELPRESSURE_HPA);
    if (isnan(altitude) || floor(altitude) == floor(-1395.92)) {
      // ALTITUDE READ FAIL
      altitude = altitude_temp;
      errorflags = errorflags | ALTIMITER_ERROR_MASK;
    } else {
      errorflags = errorflags & !ALTIMITER_ERROR_MASK;
    }
    humidity = alt->readHumidity();
  } else {
    humidity = -1;
    altitude = -1;
  }

  //skip rest of execution if there is an initialization failure
  if (errorflags & (MAGNETO_FAIL_MASK | MPU9250_FAIL_MASK))
    return;
  
  // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  if (imu->readByte(MPU9250_I2C_Address, INT_STATUS) & 0x01) {
    imu->readAccelData(imu->accelCount);  // Read the x/y/z adc values

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    imu->ax = (float)imu->accelCount[0] * imu->aRes; // - imu->accelBias[0];
    imu->ay = (float)imu->accelCount[1] * imu->aRes; // - imu->accelBias[1];
    imu->az = (float)imu->accelCount[2] * imu->aRes; // - imu->accelBias[2];

    imu->readGyroData(imu->gyroCount);  // Read the x/y/z adc values

    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    imu->gx = (float)imu->gyroCount[0] * imu->gRes;
    imu->gy = (float)imu->gyroCount[1] * imu->gRes;
    imu->gz = (float)imu->gyroCount[2] * imu->gRes;

    imu->readMagData(imu->magCount);  // Read the x/y/z adc values

    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    imu->mx = (float)imu->magCount[0] * imu->mRes
               * imu->factoryMagCalibration[0] - imu->magBias[0];
    imu->my = (float)imu->magCount[1] * imu->mRes
               * imu->factoryMagCalibration[1] - imu->magBias[1];
    imu->mz = (float)imu->magCount[2] * imu->mRes
               * imu->factoryMagCalibration[2] - imu->magBias[2];
  } // if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)

  // Must be called before updating quaternions!
  imu->updateTime();

  // Sensors x (y)-axis of the accelerometer is aligned with the y (x)-axis of
  // the magnetometer; the magnetometer z-axis (+ down) is opposite to z-axis
  // (+ up) of accelerometer and gyro! We have to make some allowance for this
  // orientationmismatch in feeding the output to the quaternion filter. For the
  // MPU-9250, we have chosen a magnetic rotation that keeps the sensor forward
  // along the x-axis just like in the LSM9DS0 sensor. This rotation can be
  // modified to allow any convenient orientation convention. This is ok by
  // aircraft orientation standards! Pass gyro rate as rad/s
  MahonyQuaternionUpdate(imu->ax, imu->ay, imu->az, imu->gx * DEG_TO_RAD,
                         imu->gy * DEG_TO_RAD, imu->gz * DEG_TO_RAD, imu->my,
                         imu->mx, imu->mz, imu->deltat);

  if (!AHRS_mode)
  {
    imu->delt_t = millis() - imu->count;
    if (imu->delt_t > 500)
    {
      if(debug && debugStream != NULL)
      {
        // Print acceleration values in milligs!
        debugStream->print("X-acceleration: "); debugStream->print(1000 * imu->ax);
        debugStream->print(" mg ");
        debugStream->print("Y-acceleration: "); debugStream->print(1000 * imu->ay);
        debugStream->print(" mg ");
        debugStream->print("Z-acceleration: "); debugStream->print(1000 * imu->az);
        debugStream->println(" mg ");

        // Print gyro values in degree/sec
        debugStream->print("X-gyro rate: "); debugStream->print(imu->gx, 3);
        debugStream->print(" degrees/sec ");
        debugStream->print("Y-gyro rate: "); debugStream->print(imu->gy, 3);
        debugStream->print(" degrees/sec ");
        debugStream->print("Z-gyro rate: "); debugStream->print(imu->gz, 3);
        debugStream->println(" degrees/sec");

        // Print mag values in degree/sec
        debugStream->print("X-mag field: "); debugStream->print(imu->mx);
        debugStream->print(" mG ");
        debugStream->print("Y-mag field: "); debugStream->print(imu->my);
        debugStream->print(" mG ");
        debugStream->print("Z-mag field: "); debugStream->print(imu->mz);
        debugStream->println(" mG");

        imu->tempCount = imu->readTempData();  // Read the adc values
        // Print temperature in degrees Centigrade
        debugStream->println(" degrees C");
      }

      imu->count = millis();
    } // if (imu->delt_t > 500)
  } // if (!AHRS)
  else
  {
    // Serial print and/or display at 0.5 s rate independent of data rates
    imu->delt_t = millis() - imu->count;

    // update LCD once per half-second independent of read rate
    if (imu->delt_t > 500)
    {
      if(debug && debugStream != NULL)
      {
        debugStream->print("ax = ");  debugStream->print((int)1000 * imu->ax);
        debugStream->print(" ay = "); debugStream->print((int)1000 * imu->ay);
        debugStream->print(" az = "); debugStream->print((int)1000 * imu->az);
        debugStream->println(" mg");

        debugStream->print("gx = ");  debugStream->print(imu->gx, 2);
        debugStream->print(" gy = "); debugStream->print(imu->gy, 2);
        debugStream->print(" gz = "); debugStream->print(imu->gz, 2);
        debugStream->println(" deg/s");

        debugStream->print("mx = ");  debugStream->print((int)imu->mx);
        debugStream->print(" my = "); debugStream->print((int)imu->my);
        debugStream->print(" mz = "); debugStream->print((int)imu->mz);
        debugStream->println(" mG");

        debugStream->print("q0 = ");  debugStream->print(*getQ());
        debugStream->print(" qx = "); debugStream->print(*(getQ() + 1));
        debugStream->print(" qy = "); debugStream->print(*(getQ() + 2));
        debugStream->print(" qz = "); debugStream->println(*(getQ() + 3));
      }

      imu->yaw   = atan2(2.0f * (*(getQ()+1) * *(getQ()+2) + *getQ()
                    * *(getQ()+3)), *getQ() * *getQ() + *(getQ()+1)
                    * *(getQ()+1) - *(getQ()+2) * *(getQ()+2) - *(getQ()+3)
                    * *(getQ()+3));
      imu->pitch = -asin(2.0f * (*(getQ()+1) * *(getQ()+3) - *getQ()
                    * *(getQ()+2)));
      imu->roll  = atan2(2.0f * (*getQ() * *(getQ()+1) + *(getQ()+2)
                    * *(getQ()+3)), *getQ() * *getQ() - *(getQ()+1)
                    * *(getQ()+1) - *(getQ()+2) * *(getQ()+2) + *(getQ()+3)
                    * *(getQ()+3));
      imu->pitch *= RAD_TO_DEG;
      imu->yaw   *= RAD_TO_DEG;
      
      // - http://www.ngdc.noaa.gov/geomag-web/#declination
      imu->yaw  += MAGNETIC_DECLINATION;
      imu->roll *= RAD_TO_DEG;

      if(debug && debugStream != NULL)
      {
        debugStream->print("Yaw, Pitch, Roll: ");
        debugStream->print(imu->yaw, 2);
        debugStream->print(", ");
        debugStream->print(imu->pitch, 2);
        debugStream->print(", ");
        debugStream->println(imu->roll, 2);

        debugStream->print("rate = ");
        debugStream->print((float)imu->sumCount / imu->sum, 2);
        debugStream->println(" Hz");
      }

      imu->count = millis();
      imu->sumCount = 0;
      imu->sum = 0;
    } // if (imu->delt_t > 500)
  } // if (AHRS)
}

float PositionSensing::getPressure() { 
  if (errorflags & (ALTIMITER_FAIL_MASK | MAGNETO_FAIL_MASK | MPU9250_FAIL_MASK)) {
    return -1;
  }
  return pressure;
}

float PositionSensing::getTemperature() { 
  if (errorflags & (ALTIMITER_FAIL_MASK | MAGNETO_FAIL_MASK | MPU9250_FAIL_MASK)) {
    return -1;
  }
  return temperature;
}

float PositionSensing::getHumidity() {
  if (errorflags & (ALTIMITER_FAIL_MASK | MAGNETO_FAIL_MASK | MPU9250_FAIL_MASK)) {
    return -1;
  }
  return humidity;
}

float PositionSensing::getPitch() {
  if (errorflags & (ALTIMITER_FAIL_MASK | MAGNETO_FAIL_MASK | MPU9250_FAIL_MASK)) {
    return -1;
  }
  return imu->pitch - zPitch;
}

float PositionSensing::getYaw() {
  if (errorflags & (ALTIMITER_FAIL_MASK | MAGNETO_FAIL_MASK | MPU9250_FAIL_MASK)) {
    return -1;
  }
  return imu->yaw - zYaw;
}

float PositionSensing::getRoll() {
  if (errorflags & (ALTIMITER_FAIL_MASK | MAGNETO_FAIL_MASK | MPU9250_FAIL_MASK)) {
    return -1;
  }
  return imu->roll - zRoll;
}

float PositionSensing::getAltitude() { 
  if (errorflags & (ALTIMITER_FAIL_MASK | ALTIMITER_ERROR_MASK))
    return -1;
  return altitude - baseline_alt; 
};

void PositionSensing::zero() { zPitch = imu->pitch; zYaw = imu->yaw; zRoll = imu->roll; };

void PositionSensing::print() {
  if (debugStream == NULL) {
    return;
  }
  StreamEx debug = *debugStream;
  debug.println("[IMU]: State Summary");
  debug.printf("       Pitch: %10.2f ˚\n", getPitch());
  debug.printf("       Yaw:   %10.2f ˚\n", getYaw());
  debug.printf("       Roll:  %10.2f ˚\n", getRoll());
  debug.printf("       Alt:   %10.2f m\n", getAltitude());
  debug.printf("       Temp:  %10.2f ˚C\n", getTemperature());
  debug.printf("       Pres:  %10.2f hPa\n", getPressure() / 100.0F);
  debug.printf("       RH:    %10.2f %%\n", getHumidity());
}
