#include "Communication.h"
#include "Photo.h"
#include "Constants.h"

CommLayer uplink(&SerialUSB, &Serial);
RoverCamera cam(PIN_CAM_SPI_CS, &Serial);
void setup() {
  Wire1.begin();
  SPI.begin();
  Serial.begin(DEBUG_BAUD);
  SerialUSB.begin(PROTOCOL_BAUD);
  cam.begin();
}

void picSzCb(unsigned int sz) {
  uplink.writeHeader(PIC, sz, M_OK);
}

void picWrtCb(byte* bytes, unsigned int len) {
  uplink.writeBodyBytes(bytes, len);
}

void (*writePicToNetFunc)(byte* bytes, unsigned int len) = &picWrtCb;
void (*picSzFunc)(unsigned int len) = &picSzCb;

void loop() {
  uplink.tick();
  if (uplink.hasNewMessage()) {
    if (uplink.getLatestMessage()->action == TAKE_PHOTO) {
      Serial.println("Photo time");
      cam.capture(picSzFunc, writePicToNetFunc);
      uplink.concludeMessage();
    }
  }
}
