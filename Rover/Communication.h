#ifndef BoR_1819_COMMUNICATION_LAYER_GUARD_H
#define BoR_1819_COMMUNICATION_LAYER_GUARD_H
#include <Stream.h>
/**
 * Layer-5 protocol for communicating between lander and controller.
 * Client-server architecture. This node is the server, and will 
 * respond to requests over a UART. This protocol assumes 
 * Layer-4 Reliable Data Transfer (Which is okay if XBEE)
 * 
 * @author Ryan D. Johnson <rdjohnson@wpi.edu>
 */

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

typedef struct {
  ActionType action;
} InboundMessage;

class CommLayer {
  public:
    CommLayer(Stream* protocstream, Stream* debugstream) : p_stream(protocstream), d_stream(debugstream), unread_msg(false) {};
    CommReturnCode tick();
    bool hasNewMessage() { return unread_msg; };
    InboundMessage* getLatestMessage() { unread_msg = false; return &msgin; };

    // Sending
    CommReturnCode writeHeader(OutgoingMessageType t, unsigned int len, OutgoingMessageStatus stat);
    CommReturnCode writeBodyBytes(unsigned char* bytes, unsigned int len);
    CommReturnCode concludeMessage();
    
  private:
    InboundMessage msgin;
    bool unread_msg;
    Stream* p_stream;
    Stream* d_stream;
    static const unsigned int MBUFF_SZ = 512;
    char mbuff[MBUFF_SZ];
    int mbuff_idx = 0;
  
};

#endif
