#include "Communication.h"
#include "Photo.h"
#include "Constants.h"
#include "Position.h"
#include "DebugDisplay.h"
#include "LandingDetection.h"
#include <Servo.h>

#define PROTOC Serial3
#define DEBUG Serial

DebugDisplay debug;
CommLayer uplink(&PROTOC, &DEBUG, &debug);
RoverCamera cam(PIN_CAM_SPI_CS, &DEBUG, &debug);
PositionSensing pos(&DEBUG, &debug);
LandingDetection landingDetect(&pos, &DEBUG, &debug);

//Servo test;

void setup() {
  DEBUG.begin(DEBUG_BAUD);
  PROTOC.begin(PROTOCOL_BAUD);
  DEBUG.println("[INIT] Beginning Power-on Set-Up and Self-Tests");
  debug.begin();
  Wire1.begin();
  SPI.begin();
  cam.begin();
  pos.begin();
  //test.attach(22);
  DEBUG.println("[INIT] Completed");
  debug.setChar("run");
}

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

void loop() {
  pos.tick();
  landingDetect.tick();
  uplink.tick();
  //pos.print();
  debug.tick();
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

      // Runtime
      String body = "Time: ";
      body += millis();
      body += "\n";
      
      // ALT
      body += "Altitude: ";
      body += pos.getAltitude();
      body += "\n";

      // LAND STATE
      body += "Landing State: ";
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

      uplink.writeHeader(STATUS, body.length() + 1, M_OK); //write OK message with no body
      uplink.writeBodyBytes((unsigned char*)body.c_str(), body.length() + 1);
      uplink.concludeMessage();
    } else if (uplink.getLatestMessage()->action == SOIL_SAMPLE) {
      uplink.writeHeader(STATUS, 0, M_OK); //write OK message with no body
      uplink.concludeMessage();
    } else if (uplink.getLatestMessage()->action == PING) {
      uplink.writeHeader(PONG, 4, M_OK);
      uplink.writeBodyBytes((unsigned char *)"pong", 4);
      uplink.concludeMessage();
    }
  }
}
