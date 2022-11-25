#include <Adafruit_Fingerprint.h>
#include <KnxTelegram.h>
#include <KnxTpUart.h>


#define SENSOR_TX 6                  //GPIO Pin for RX
#define SENSOR_RX 5                  //GPIO Pin for TX


#define PHYSICAL_ADDRESS "1.1.250"

#define MatchGroupAddress "0/0/0"
#define LearnGroupAddress "0/0/0"
#define DeleteGroupAddress "0/0/0"
#define LastIDGroupAddress "0/0/0"
#define LastConfidenceScoreGroupAddress "0/0/0"
#define DoorlockGroupAddress "0/0/0"


SoftwareSerial mySerial(SENSOR_TX, SENSOR_RX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


KnxTpUart knx(&Serial, PHYSICAL_ADDRESS);


uint8_t id = 0;                       //Stores the current fingerprint ID
uint8_t lastID = 0;                   //Stores the last matched ID
uint8_t lastConfidenceScore = 0;      //Stores the last matched confidence score
boolean modeLearning = false;
boolean modeReading = true;
boolean modeDelete = false;






void setup()
{
 
  //Serial.begin(57600);
  Serial.begin(19200, SERIAL_8E1);
  knx.uartReset();

  knx.addListenGroupAddress(LearnGroupAddress);
  knx.addListenGroupAddress(DeleteGroupAddress);
  
  while (!Serial);
  delay(100);


  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
  } else {
//    finger.LEDcontrol(1,100,2,1); // code(1-6),speed(1-255),color(1-R/2-B/3_P),time(1-255)
    while (1) {
      delay(1);
    }
  }
}

void loop() {
  
  if (modeReading == true && modeLearning == false) {
    uint8_t result = getFingerprintID();
    if (result == FINGERPRINT_OK) {
      knx.groupWriteBool(MatchGroupAddress, true); //match
      knx.groupWriteBool(DoorlockGroupAddress, true);
      delay(500);
    } else if (result == FINGERPRINT_NOTFOUND) {
      knx.groupWriteBool(MatchGroupAddress, false); //match
      delay(500);
    } else if (result == FINGERPRINT_NOFINGER) {
    } else {
    }
  }
  delay(100);            //don't need to run this at full speed.
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_NOFINGER:
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      return p;
    case FINGERPRINT_IMAGEFAIL:
      return p;
    default:
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_IMAGEMESS:
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      return p;
    case FINGERPRINT_FEATUREFAIL:
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      return p;
    default:
      return p;
  }

  // OK converted!
  
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 150, FINGERPRINT_LED_BLUE, 1);
    lastID = finger.fingerID;
    lastConfidenceScore = finger.confidence;
        knx.groupWrite1ByteInt(LastIDGroupAddress, lastID);
        knx.groupWrite1ByteInt(LastConfidenceScoreGroupAddress, lastConfidenceScore);
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
      finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 150, FINGERPRINT_LED_RED, 1);
    return p;
  } else {
   //
    return p;
  }




  return finger.fingerID;
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
            finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 100, FINGERPRINT_LED_BLUE, 1);
            delay(1000);
        break;
      case FINGERPRINT_NOFINGER:
       finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 100, FINGERPRINT_LED_PURPLE);
       delay(1000);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        break;
      case FINGERPRINT_IMAGEFAIL:
        break;
      default:
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_IMAGEMESS:
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      return p;
    case FINGERPRINT_FEATUREFAIL:
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      return p;
    default:
      return p;
  }

  //Serial.println("Remove finger");
  delay(1000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
            finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 100, FINGERPRINT_LED_BLUE, 3);
        break;
      case FINGERPRINT_NOFINGER:
            finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 100, FINGERPRINT_LED_PURPLE);
            delay(1000);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        break;
      case FINGERPRINT_IMAGEFAIL:
        break;
      default:
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_IMAGEMESS:
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      return p;
    case FINGERPRINT_FEATUREFAIL:
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      return p;
    default:
      return p;
  }

  // OK converted!

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    return p;
  } else {
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    return true;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    return p;
  } else {
    return p;
  }
}

uint8_t deleteFingerprint() {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
      finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 25, FINGERPRINT_LED_RED, 3);
    return true;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    return p;
  } else {
    return p;
  }
}





void serialEvent() {
  KnxTpUartSerialEventType eType = knx.serialEvent();
  if (eType == TPUART_RESET_INDICATION) {
  } 
  else if (eType == KNX_TELEGRAM) {
    KnxTelegram* telegram = knx.getReceivedTelegram();
    String target =
      String(0 + telegram->getTargetMainGroup())   + "/" +
      String(0 + telegram->getTargetMiddleGroup()) + "/" +
      String(0 + telegram->getTargetSubGroup());

    if (telegram->getCommand() == KNX_COMMAND_WRITE) {
     
      if (target == LearnGroupAddress) {
        int received_enroll_id = telegram->get1ByteIntValue();
        id = received_enroll_id;
        
        if (id > 0 && id < 128) {

            while (!getFingerprintEnroll());

                modeLearning = false;
                modeReading = true;
                modeDelete = false;
                id = 0;
      }  
      }

            if (target == DeleteGroupAddress) {
        int received_delete_id = telegram->get1ByteIntValue();
        id = received_delete_id;
        
        if (id > 0 && id < 128) {

            while (! deleteFingerprint());

                modeLearning = false;
                modeReading = true;
                modeDelete = false;
                id = 0;
      }
      }
    }
  }
}
