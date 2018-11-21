#include "Communication.h"
#include "Photo.h"
#define usbserial
/*
ArduCAM myCAM(OV2640, 10); //10 is the SPI Chip Select Pin
uint8_t vid, pid;

#define MBUFF_SZ 512
char mbuff[MBUFF_SZ];
byte buf[256];
uint8_t temp = 0,temp_last = 0;
bool is_header = false;
uint32_t length = 0;

void myCAMSSerialWrite(){
  char str[8];
  byte buf[256];
  static int i = 0;
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  bool is_header = false;
  File outFile;
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("start Capture."));
  while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
  Serial.println(F("Capture Done."));  
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length);

  #ifndef usbserial
  Serial3.print("Content-Length: ");
  Serial3.println(length);
  Serial3.println("Content-Type: image/jpeg");
  Serial3.println("Status: 200 OK");
  Serial3.print("\r\n");
  #endif
  
  if (length >= MAX_FIFO_SIZE) 
  {
    Serial.println(F("Over size."));
    return ;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
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
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buf[i++] = temp;  //save the last  0XD9     
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      #ifdef usbserial
      SerialUSB.write(buf, i); 
      SerialUSB.print("\r\n\r\n\r\n");
      #endif
      #ifndef usbserial
      Serial3.write(buf, i);
      Serial3.print("\r\n\r\n\r\n");
      #endif  
      //Close the file
      Serial.println(F("Image Save Done!"));
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
        #ifdef usbserial
        SerialUSB.write(buf, 256); 
        #endif
        #ifndef usbserial
        Serial3.write(buf, 256); 
        #endif
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

void setup() {
  Wire1.begin();
  SPI.begin();
  Serial.begin(9600);
  testComm();
  #ifdef usbserial
  Serial.println("Waiting for USB serial device on native port");
  SerialUSB.begin(115200);
  while(!SerialUSB);
  #endif
  #ifndef usbserial
  Serial3.begin(115200);
  Serial.println("Waiting for UART on TX3/RX3 (XBEE)");
  while(!Serial3);
  #endif
  // put your setup code here, to run once:
  while(true) {
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
      Serial.println(F("Can't find OV2640 module!"));
      delay(1000);continue;
    }else{
      Serial.println(F("OV2640 detected."));break;
    }
  }
  while(true){
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
      Serial.println(F("ACK CMD SPI interface Error!"));
      delay(1000);continue;    
    } else {
      Serial.println(F("ACK CMD SPI interface OK.")); break;
    }
  }
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_1600x1200); //OV2640_640x480, 1600x1200
}


int mbuff_idx = 0;
void loop() {
  bool gotmsg = false;

  while (Serial3.available()) {
    mbuff[mbuff_idx++] = Serial3.read();
  }
  if (mbuff[mbuff_idx-1] == '\n' && mbuff[mbuff_idx-2] == '\r' && 
      mbuff[mbuff_idx-3] == '\n' && mbuff[mbuff_idx-4] == '\r' && mbuff_idx > 4 ) {
    Serial.println("end of message");
    Serial.println(mbuff_idx);
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
          Serial.print(field);
          Serial.print(": ");
          Serial.println(value);
          if (strncmp(field, "Action", 6) == 0) {
            // Action command
            if (strncmp(value, "photo", 5) == 0) {
              //take a photo
              Serial.println("Will take photo");
              myCAMSSerialWrite();
            }
          }
          break;
        }
      }
    }
    mbuff_idx = 0;
  }
}*/
CommLayer uplink(&SerialUSB, &Serial);
RoverCamera cam(10, &Serial);
void setup() {
  Wire1.begin();
  SPI.begin();
  Serial.begin(9600);
  SerialUSB.begin(2000000);
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
