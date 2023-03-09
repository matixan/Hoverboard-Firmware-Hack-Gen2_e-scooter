
/*
// Variables todo
uint8_t upperLEDMaster = 0;
uint8_t lowerLEDMaster = 0;
uint8_t mosfetOutMaster = 0;
uint8_t upperLEDSlave = 0;
uint8_t lowerLEDSlave = 0;
uint8_t mosfetOutSlave = 0;
uint8_t beepsBackwards = 0;
uint8_t activateWeakening = 0;
*/

#define START_FRAME         0xABCD       // [-] Start frme definition for reliable serial communication

typedef struct{
   uint16_t cStart = START_FRAME;    //  = '/';
   int16_t iSpeedL;   // 100* km/h
   int16_t iSpeedR;   // 100* km/h  not yet supported
   uint16_t iVolt;    // 100* V
   int16_t iAmpL;   // 100* A
   int16_t iAmpR;   // 100* A       not yet supported
   uint16_t checksum;
} SerialHover2Server;

typedef struct{
   uint8_t cStart = '/';;
   int16_t  iSpeed = 0;
   int16_t  iSteer = 0;
   uint16_t checksum;
   uint8_t cEnd = '\n';
} SerialServer2Hover;

uint16_t CalcCRC(uint8_t *ptr, int count)
{
  uint16_t  crc;
  uint8_t i;
  crc = 0;
  while (--count >= 0)
  {
    crc = crc ^ (uint16_t) *ptr++ << 8;
    i = 8;
    do
    {
      if (crc & 0x8000)
      {
        crc = crc << 1 ^ 0x1021;
      }
      else
      {
        crc = crc << 1;
      }
    } while(--i);
  }
  return (crc);
}


template <typename O,typename I> void HoverSetupEsp32(O& oSerial, I iBaud, I pin_RX, I pin_TX)
{
  // Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  oSerial.begin(iBaud, SERIAL_8N1, pin_RX, pin_TX);  //Serial1 = 0, 4   Serial2 = 16,17;
}
template <typename O,typename I> void HoverSetupArduino(O& oSerial, I iBaud)
{
  oSerial.begin(iBaud);
}


void DebugOut(uint8_t aBuffer[], uint8_t iSize)
{
  for (int i=0; i<iSize; i++)
  {
    uint8_t c = aBuffer[i];
    Serial.print((c < 16) ? " 0" : " ");Serial.print(c,HEX); 
  }
  Serial.println();
}


// ########################## SEND ##########################
//void Send(Serial& oSerial, int16_t uSteer, int16_t uSpeed)
template <typename O,typename U> void HoverSend(O& oSerial, U uSteer, U uSpeed)
{
  //DEBUGT("uSteer",uSteer);DEBUGLN("uSpeed",uSpeed);
  SerialServer2Hover oData;
  oData.iSpeed    = (int16_t)uSpeed;
  oData.iSteer    = (int16_t)uSteer;
  oData.checksum = CalcCRC((uint8_t*)&oData, 5);
  oSerial.write((uint8_t*) &oData, sizeof(oData)); 
  //DebugOut((uint8_t*) &oData, sizeof(oData)); 

}
template <typename O,typename I> void HoverSendLR(O& oSerial, I iSpeedLeft, I iSpeedRight) // -1000 .. +1000
{
  // speed coeff in config.h must be 1.0 : (DEFAULT_)SPEED_COEFFICIENT   16384
  // steer coeff in config.h must be 0.5 : (DEFAULT_)STEER_COEFFICIENT   8192 
  HoverSend(oSerial,iSpeedRight - iSpeedLeft,(iSpeedLeft + iSpeedRight)/2);
}

void HoverLog(SerialHover2Server& oData)
{
  DEBUGT("iSpeedL",(float)oData.iSpeedL/100.0);
  //DEBUGT("iSpeedR",(float)oData.iSpeedR/100.0);
  DEBUGT("iAmpL",(float)oData.iAmpL/100.0);
  //DEBUGT("iAmpR",(float)oData.iAmpR/100.0);
  DEBUGLN("iVolt",(float)oData.iVolt/100.0);
  //Serial.print("\tcrc="); Serial.println(oData.checksum,HEX);
}

#ifdef DEBUG_RX
  unsigned long iLastRx = 0;
#endif

//boolean Receive(Serial& oSerial, SerialFeedback& Feedback)
template <typename O,typename OF> boolean Receive(O& oSerial, OF& Feedback)
{
  int iAvail = oSerial.available() - sizeof(SerialHover2Server);
  int8_t iFirst = 1;
  while (iAvail >= 3+iFirst )
  {
    byte c = oSerial.read();  // Read the incoming byte
    iAvail--;

    #ifdef DEBUG_RX
      if (millis() > iLastRx + 50)  Serial.println();
      Serial.print((c < 16) ? " 0" : " ");
      Serial.print(c,HEX); 
      iLastRx = millis();
    #endif
    
    if (iFirst) // test first START byte
    {
      if (c == (byte)START_FRAME) //if (c == 0xCD)
      {
        iFirst = 0;
      }
    }
    else  // test second START byte
    {
      if (c == START_FRAME >>8 ) //if (c == 0xAB)
      {
        SerialHover2Server tmpFeedback;
        byte* p = (byte *)&tmpFeedback+2; // start word already read
        for (int i = sizeof(SerialHover2Server); i>2; i--)  
          *p++    = oSerial.read();

        #ifdef DEBUG_RX
          Serial.print(" -> ");
          HoverLog(tmpFeedback);
        #endif

        uint16_t checksum = CalcCRC((byte *)&tmpFeedback, sizeof(SerialHover2Server)-2);
        if (checksum == tmpFeedback.checksum)
        {
            memcpy(&Feedback, &tmpFeedback, sizeof(SerialHover2Server));
            #ifdef DEBUG_RX
              Serial.println(" :-))))))))))");
            #endif
            return true;
        }
        #ifdef DEBUG_RX
          Serial.print(tmpFeedback.checksum, HEX);
          Serial.print(" != ");
          Serial.print(checksum,HEX);
          Serial.println(" :-(");
        #endif
        return false;       
      }
      if (c != (byte)START_FRAME) //if (c != 0xCD)
        iFirst = 1;
    }
  }
  return false;
}
