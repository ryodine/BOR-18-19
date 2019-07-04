/**
 * Layer-5 protocol for communicating between lander and controller.
 * Client-server architecture. This node is the server, and will 
 * respond to requests over a UART. This protocol assumes 
 * Layer-4 Reliable Data Transfer (Which is okay if XBEE)
 * 
 * @author Ryan D. Johnson
 * 
 * WPI Battle of the Rockets 2018-2019
 * Team rocket Powered Locomotive
 */

#include "Communication.h"

CommReturnCode CommLayer::tick() {
  bool gotmsg = false;
  while (p_stream->available()) {
    mbuff[mbuff_idx++] = p_stream->read();
  }
  if (mbuff[mbuff_idx-1] == '\n' && mbuff[mbuff_idx-2] == '\r' && 
      mbuff[mbuff_idx-3] == '\n' && mbuff[mbuff_idx-4] == '\r' && mbuff_idx > 4 ) {
    d_stream->println("[COMM] Got a (completed) message. Headers:");
    char field[20];
    char value[20];
    int index = 0;
    int index2 = 0;
    bool fieldmode = true;
    while (true) {
      if (fieldmode) {
        if (mbuff[index] != ':') {
          field[index++] = mbuff[index2++];
        } else {
          field[index] = '\0';
          index2 += 2;
          fieldmode = false;
          index = 0;
        }
      } else {
        if (index2 < mbuff_idx) {
          if (mbuff[index2] != '\r' && mbuff[index2] != '\n') {
            value[index++] = mbuff[index2++];
          } else {
            index2++;
          }
          
        } else {
          value[index] = '\0';
          d_stream->print("  =>");
          d_stream->print(field);
          d_stream->print(": ");
          d_stream->println(value);
          if (strncmp(field, "Action", 6) == 0) {
            // Action command
            if (strncmp(value, "photo", 5) == 0) {
              //take a photo
              msgin.action = TAKE_PHOTO;
              unread_msg = true;
            } else if (strncmp(value, "status", 6) == 0) {
              //retreive status
              msgin.action = GET_STAT;
              unread_msg = true;
            } else if (strncmp(value, "soil", 4) == 0) {
              //retreive status
              msgin.action = SOIL_SAMPLE;
              unread_msg = true;
            } else if (strncmp(value, "ping", 4) == 0) {
              //retreive status
              msgin.action = PINGM;
              unread_msg = true;
            } else if (strncmp(value, "arm", 3) == 0) {
              msgin.action = ARM;
              unread_msg = true;
            } else if (strncmp(value, "zero", 3) == 0) {
              msgin.action = ZERO_LEGS;
              unread_msg = true;
            } else if (strncmp(value, "land", 3) == 0) {
              msgin.action = MANUAL_LAND;
              unread_msg = true;
            }
          }
          break;
        }
      }
    }
    mbuff_idx = 0;
  }
  return OK;
}

CommReturnCode CommLayer::writeHeader(OutgoingMessageType t, unsigned int len, OutgoingMessageStatus stat) {
  p_stream->print("Content-Length: ");
  p_stream->println(len);
  
  switch (t) {
    case PIC:
      p_stream->println("Content-Type: image/jpeg");
      break;
    default:
      p_stream->println("Content-Type: text/plain");
  }

  switch (stat) {
    case M_OK:
      p_stream->println("Status: 200 OK");
      break;
    case M_BUSY:
      p_stream->println("Status: 503 BUSY");
      break;
    case M_UNPARSEABLE:
      p_stream->println("Status: 400 UNPROCESSABLE");
      break;
    case M_ERR:
    default:
      p_stream->println("Status: 500 ERROR");
  }

  p_stream->print("\r\n");
  d_stream->println("[COMM] Began Message. Wrote Message Header");
  return OK;
}
CommReturnCode CommLayer::writeBodyBytes(unsigned char* bytes, unsigned int len) {
  p_stream->write(bytes, len);
  return OK;
}
CommReturnCode CommLayer::concludeMessage() {
  p_stream->println("\r\n\r\n\r\n");
  d_stream->println("[COMM] Outgoing Message complete");
  return OK;
}
