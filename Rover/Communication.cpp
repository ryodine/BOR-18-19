#include "Communication.h"

CommReturnCode CommLayer::tick() {
  bool gotmsg = false;
  while (p_stream->available()) {
    mbuff[mbuff_idx++] = p_stream->read();
  }
  if (mbuff[mbuff_idx-1] == '\n' && mbuff[mbuff_idx-2] == '\r' && 
      mbuff[mbuff_idx-3] == '\n' && mbuff[mbuff_idx-4] == '\r' && mbuff_idx > 4 ) {
    d_stream->println("end of message");
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
          d_stream->print(field);
          d_stream->print(": ");
          d_stream->println(value);
          if (strncmp(field, "Action", 6) == 0) {
            // Action command
            if (strncmp(value, "photo", 5) == 0) {
              //take a photo
              msgin.action = TAKE_PHOTO;
              unread_msg = true;
              //myCAMSSerialWrite();
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
    case ERR:
    default:
      p_stream->println("Content-Type: text/plain");
  }

  switch (stat) {
    case M_OK:
      p_stream->println("Status: 200 OK");
      break;
    case M_ERR:
    default:
      p_stream->println("Status: 500 ERROR");
  }

  p_stream->print("\r\n");
  d_stream->println("Started message");
  return OK;
}
CommReturnCode CommLayer::writeBodyBytes(unsigned char* bytes, unsigned int len) {
  p_stream->write(bytes, len);
  return OK;
}
CommReturnCode CommLayer::concludeMessage() {
  p_stream->println("\r\n\r\n\r\n");
  d_stream->println("Outgoing Message complete");
  return OK;
}
