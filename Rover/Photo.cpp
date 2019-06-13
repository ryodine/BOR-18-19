/**
 * Take photos, and get the bytes.
 * 
 * @author Ryan D. Johnson
 * 
 * WPI Battle of the Rockets 2018-2019
 * Team rocket Powered Locomotive
 */

#include "Photo.h"

void RoverCamera::begin() {
  uint8_t temp = 0, vid = 0, pid = 0;
  while(true) {
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
      if (debugStream != NULL)
        debugStream->println(F("[INIT: Camera] Can't find OV2640 module!"));

        //STOP CODE
        //debugDisplay->setStopCode("CI2C");

        
      delay(1000);continue;
    }else{
      if (debugStream != NULL)
        debugStream->println(F("[INIT: Camera] OV2640 detected."));break;
    }
  }
  while(true){
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
      if (debugStream != NULL)
        debugStream->print(F("[INIT: Camera] Error communicating with SPI. Did not recieve ACK. "));
        debugStream->println(temp);

        //STOP CODE
        //debugDisplay->setStopCode("CSPI");

      delay(1000);continue; 
         
    } else {
      if (debugStream != NULL)
        debugStream->println(F("[INIT: Camera] ACK CMD SPI interface OK.")); break;
    }
  }
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_640x480);
}

/**
 * Copied and Modified from ArduCam example code. Super ugly, but seems to work.
 * 
 * Wrapped the capturing logic in this function, which has two callbacks: 
 * - One (first) gets executed when the image size is first discovered
 * - The other one (Second) gets executed every time it reads a chunk of
 *   bytes from the camera's FIFO.
 * - The function returns when there are no more bytes to handle in callbacks.
 * 
 * Callbacks are used so that extra memory allocation doesn't need to happen.
 * In this design, the system does NOT store-and-forward, and can run in
 * memory-constrained environments.
 */

void RoverCamera::reset() {
  myCAM.flush_fifo();
  myCAM.CS_HIGH();
}

void RoverCamera::capture(void (*szcallback)(unsigned int len), void (*bytecallback)(byte* bytes, unsigned int len)) {
  char str[8];
  byte buf[256];
  static int i = 0;
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  bool is_header = false;
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  if (debugStream != NULL)
    debugStream->println(F("Start Capture."));
  while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK)) {
    delay(100);
    debugStream->print(F("."));
  }
  debugStream->println(F(""));
  if (debugStream != NULL)
    debugStream->println(F("Capture Done."));  
  length = myCAM.read_fifo_length();
  if (debugStream != NULL) {
    debugStream->print(F("The fifo length is: "));
    debugStream->println(length);
  }

  //Calback length
  (*szcallback) (length);
  
  if (length >= MAX_FIFO_SIZE) 
  {
    if (debugStream != NULL)
      debugStream->println(F("Over size."));
    return ;
  }
  if (length == 0 ) //0 kb
  {
    if (debugStream != NULL)
      debugStream->println(F("Size is 0."));
    return ;
  }
  i = 0;
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  //Read JPEG data from FIFO
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end, break while
    {
      buf[i++] = temp;  //save the last  0XD9     
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();

      //Calback bytes
      (*bytecallback) (buf, i); 

      
      //Close the file
      if (debugStream != NULL)
        debugStream->println(F("Image Save Done!"));
      is_header = false;
      i = 0;
    }  
    if (is_header == true)
    { 
      //Write image data to buffer if not full
      if (i < 256)
        buf[i++] = temp;
      else
      {
        //Write 256 bytes image data to file
        myCAM.CS_HIGH();
        
        
        //Calback bytes
        (*bytecallback) (buf, 256); 

        
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }        
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;   
    } 
  }
}
