#ifndef BoR_1819_COMMUNICATION_LAYER_GUARD_H
#define BoR_1819_COMMUNICATION_LAYER_GUARD_H

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

#include <Stream.h>
#include "Constants.h"
#include "DebugDisplay.h"

typedef struct {
  ActionType action;
} InboundMessage;

class CommLayer {
  public:
    CommLayer(Stream* protocstream, Stream* debugstream, DebugDisplay* debugdisplay) : p_stream(protocstream), d_stream(debugstream), debugDisplay(debugdisplay), unread_msg(false) {};
    CommReturnCode tick();
    bool hasNewMessage() { return unread_msg; };
    InboundMessage* getLatestMessage() { unread_msg = false; return &msgin; };
    InboundMessage* peekLatestMessage() { return &msgin; };
    // Sending
    CommReturnCode writeHeader(OutgoingMessageType t, unsigned int len, OutgoingMessageStatus stat);
    CommReturnCode writeBodyBytes(unsigned char* bytes, unsigned int len);
    CommReturnCode concludeMessage();
    
  private:
    InboundMessage msgin;
    bool unread_msg;
    Stream* p_stream;
    Stream* d_stream;
    DebugDisplay* debugDisplay;
    static const unsigned int MBUFF_SZ = 512;
    char mbuff[MBUFF_SZ];
    int mbuff_idx = 0;
  
};

#endif
