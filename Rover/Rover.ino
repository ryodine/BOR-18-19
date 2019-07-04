#include "Communication.h"
#include "Photo.h"
#include "Constants.h"
#include "Position.h"
#include "DebugDisplay.h"
#include "LandingDetection.h"
#include "Drivetrain.h"
#include <Servo.h>

#define PROTOC Serial3
#define DEBUG Serial


DebugDisplay debug;
CommLayer uplink(&PROTOC, &DEBUG, &debug);
RoverCamera cam(PIN_CAM_SPI_CS, &DEBUG, &debug);
PositionSensing pos(&DEBUG, &debug);
LandingDetection landingDetect(&pos, &DEBUG, &debug);
Drivetrain drive;

unsigned long error_flasher = 0;
bool error_light_on = false;


void setup() {
  DEBUG.begin(DEBUG_BAUD);
  PROTOC.begin(PROTOCOL_BAUD);
  pinMode(13, OUTPUT);

  //Initialization
  DEBUG.println(F("[INIT] Beginning Power-on Set-Up and Self-Tests"));
  debug.begin();
  Wire.begin();
  SPI.begin();
  cam.begin();
  pos.begin();
  landingDetect.begin();
  //drive.setup();
  //drive.getRightWheel().setRPM(20);
  
  DEBUG.println("[INIT] Completed");
  debug.setChar("run");

  //drive.zero();
  //drive.getRightWheel().setRPM(20);
  //drive.getRightWheel().CommutateTo(Wheel::RPM);
}

// Camera photo callbacks
void picSzCb(unsigned int sz) {
  uplink.writeHeader(PIC, sz, M_OK);
}
void picWrtCb(byte* bytes, unsigned int len) {
  uplink.tick(); //make sure to check for preemptive messages
  if (uplink.hasNewMessage()) {
    cam.reset();
    DEBUG.println("RESET");
  } else {
    DEBUG.print(".");
    debug.tick();
    uplink.writeBodyBytes(bytes, len);
  }
}
void (*writePicToNetFunc)(byte* bytes, unsigned int len) = &picWrtCb;
void (*picSzFunc)(unsigned int len) = &picSzCb;
int l = 1;
void loop() {
  //Ticks
  pos.tick();
  landingDetect.tick();
  uplink.tick();
  debug.tick();
  //drive.tick();
  
  //Error Flasher
  if (pos.hasError() && ((millis() - error_flasher) > 100)) {
    error_flasher = millis();
    error_light_on = !error_light_on;
    digitalWrite(13, error_light_on);
  }
  
  //Message handling
  if (uplink.hasNewMessage()) {
    if (uplink.getLatestMessage()->action == TAKE_PHOTO) {
      DEBUG.println("Photo time");
      debug.setChar("0000");
      cam.capture(picSzFunc, writePicToNetFunc);
      uplink.concludeMessage();
      if (uplink.peekLatestMessage()->action == TAKE_PHOTO) {
        uplink.getLatestMessage(); //if there is a backed up photo request, get rid of it
      }
      debug.setChar("run");
    } else if (uplink.getLatestMessage()->action == GET_STAT) {      
      
      // ALT
      String body = "Altitude: ";
      body += pos.getAltitude();
      body += "\n";

      body += "POS STATUS: ";
      body += pos.getErrorFlags();
      body += "\n";

      // LAND STATE
      body += "Landing State: ";
      if (landingDetect.isArmed()) {
        body += "(ARMED) ";
      } else {
        body += "(UNARMED) ";
      }
      switch (landingDetect.getState()) {
        case NO_TAKEOFF:
          body += "Not yet taken off.";
          break;
        case ASCENDING:
          body += "Ascending.";
          break;
        case DESCENDING:
          body += "Descending.";
          break;
        case LANDING_WAIT:
          body += "Waiting for landed timeout to expire.";
          break;
        case LANDED:
          body += "Landed.";
          break;
      }

      body += "\nmillis: ";
      body += String(millis()) + "\n";

      uplink.writeHeader(STATUS, body.length() + 1, M_OK); //write OK message with no body
      uplink.writeBodyBytes((unsigned char*)body.c_str(), body.length() + 1);
      uplink.concludeMessage();
    } else if (uplink.getLatestMessage()->action == SOIL_SAMPLE) {
      delay(1000);
      uplink.writeHeader(STATUS, 8, M_OK); //write OK message with no body
      /*if (drive.getControlMode() == Drivetrain::DCR_STOP) {
        drive.setSynchronizedRPM(20);
      }*/
      uplink.writeBodyBytes((unsigned char *)"soil req", 9);
      uplink.concludeMessage();
    } else if (uplink.getLatestMessage()->action == PINGM) {
      uplink.writeHeader(PONG, 4, M_OK);
      uplink.writeBodyBytes((unsigned char *)"pong", 5);
      uplink.concludeMessage();
    } else if (uplink.getLatestMessage()->action == ARM) {
      landingDetect.armPhoto();
      uplink.writeHeader(ARMED, 2, M_OK);
      uplink.writeBodyBytes((unsigned char *)"ok", 3);
      uplink.concludeMessage();
    } else if (uplink.getLatestMessage()->action == MANUAL_LAND) {
      uplink.writeHeader(COMMAND_ACK, 2, M_OK);
      uplink.writeBodyBytes((unsigned char *)"ok", 3);
      uplink.concludeMessage();
    } else if (uplink.getLatestMessage()->action == ZERO_LEGS) {
      uplink.writeHeader(COMMAND_ACK, 2, M_OK);
      uplink.writeBodyBytes((unsigned char *)"ok", 3);
      uplink.concludeMessage();
    } else {
      uplink.writeHeader(COMMAND_ACK, 2, M_UNPARSEABLE);
      uplink.writeBodyBytes((unsigned char *)"unknown", 8);
      uplink.concludeMessage();
    }
  }
}
