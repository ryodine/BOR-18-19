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

//! Controllers
DebugDisplay debug;
CommLayer uplink(&PROTOC, &DEBUG, &debug);
RoverCamera cam(PIN_CAM_SPI_CS, &DEBUG, &debug);
PositionSensing pos(&DEBUG, &debug);
LandingDetection landingDetect(&pos, &DEBUG, &debug);
Drivetrain drive;

//! Error Flasher states
unsigned long error_flasher = 0;
bool error_light_on = false;

//! PRS & Vacuum Servo
Servo prs;

// ================================================================ Setup =====
void setup() {
  DEBUG.begin(DEBUG_BAUD);
  PROTOC.begin(PROTOCOL_BAUD);
  pinMode(13, OUTPUT);

  DEBUG.println(F("[INIT] Beginning Power-on Set-Up and Self-Tests"));

  //! Begin Controllers
  Wire.begin();
  debug.begin();
  debug.write("Starting");
  SPI.begin();
  cam.begin();
  pos.begin();
  landingDetect.begin();
  drive.setup();

  //! PRS
  prs.attach(PRS_PWM);
  prs.write(PRS_CLOSE_VALUE);
  
  pinMode(VAC_PIN, OUTPUT);

  //drive.getRightWheel().setRPM(20);
  
  DEBUG.println("[INIT] Completed");
  debug.setChar("Running");


  //drive.zero();
  //drive.getRightWheel().setRPM(20);
  //drive.getRightWheel().CommutateTo(Wheel::RPM);
}

// ======================================================= Camera Actions =====
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

//! While this is a blockign method, it can be prematurely exited it it gets a 
//! new message request
void photoActionBlocking() {
  DEBUG.println("Photo time");
  debug.write("PHOTO");
  cam.capture(picSzFunc, writePicToNetFunc);
  uplink.concludeMessage();
  if (uplink.peekLatestMessage()->action == TAKE_PHOTO) {
    // if there is a backed up photo request, get rid of it
    uplink.getLatestMessage(); 
  }
}

// ================================================== Soil Sample Routine =====
void soilSampleBlocking() {
  uplink.writeHeader(STATUS, 8, M_OK);
  uplink.writeBodyBytes((unsigned char *)"soil req", 9);
  uplink.concludeMessage();
  debug.write("SOIL ROUTINE");
  digitalWrite(VAC_PIN, HIGH);
  delay(5000);
  digitalWrite(VAC_PIN, LOW);
}

// ================================================== Leg Zeroing Routine =====
void zeroLegRoutineBlocking() {
  drive.zero();
  drive.tick();
  while (drive.getControlMode() != Drivetrain::DCR_STOP) {
    if (uplink.hasNewMessage()) {
      //If new message, break.
      return;
    }
    drive.tick();
  }
}

// ======================================================= Landed Routine =====
double rotations_begin = 0.0;
void landedRoutineBlocking() {
  debug.write("LANDING ROUTINE ");
  rotations_begin = drive.getLeftWheel().getRotations();
  prs.write(PRS_OPEN_VALUE);
  drive.setSynchronizedRPM(FW_RPM);
  drive.tick();
  while (drive.getControlMode() != Drivetrain::DCR_STOP) {
    delay(5);
    if (uplink.hasNewMessage() || 
       ((drive.getLeftWheel().getRotations() - rotations_begin) > LANDED_FORWARD_STEPS)) {
      drive.stop();
      drive.tick();
      return;
    }
    drive.tick();
  }
}

// ========================================================= Status Query =====
void statusQueryBlocking() {
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
}

// ================================================= Main Controller Loop =====
int l = 1;
void loop() {
  // Ticks
  pos.tick();
  landingDetect.tick();
  uplink.tick();
  debug.tick();
  drive.tick();

  // Watch for landing:
  if (landingDetect.getState() == LANDED) {
    landedRoutineBlocking();
  }
  
  // Error Flasher
  if (pos.hasError() && ((millis() - error_flasher) > 100)) {
    error_flasher = millis();
    error_light_on = !error_light_on;
    digitalWrite(13, error_light_on);
  } else if (! pos.hasError()) {
    digitalWrite(13, HIGH);
  }
  
  // Message handling
  if (uplink.hasNewMessage()) {
    debug.write((String("Proc Msg #") + String(uplink.getLatestMessage()->action)).c_str());
    if (uplink.getLatestMessage()->action == TAKE_PHOTO) {
      photoActionBlocking();
    } else if (uplink.getLatestMessage()->action == GET_STAT) {      
      statusQueryBlocking();
    } else if (uplink.getLatestMessage()->action == SOIL_SAMPLE) {
      soilSampleBlocking();
    } else if (uplink.getLatestMessage()->action == PINGM) {
      // Simple pong response
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
      landedRoutineBlocking();
    } else if (uplink.getLatestMessage()->action == ZERO_LEGS) {
      uplink.writeHeader(COMMAND_ACK, 2, M_OK);
      uplink.writeBodyBytes((unsigned char *)"ok", 3);
      uplink.concludeMessage();
      zeroLegRoutineBlocking();
    } else {
      uplink.writeHeader(COMMAND_ACK, 2, M_UNPARSEABLE);
      uplink.writeBodyBytes((unsigned char *)"?? unknown command", 18);
      uplink.concludeMessage();
    }
    debug.write((String("Proc Msg #") + String(uplink.getLatestMessage()->action) + " Done").c_str());
  }
}
