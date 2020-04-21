/***************************************************
 Heavily modified by SwitchDoc Labs for reliablity
 February 2018 and in 2017



 This is a library for the AM2315 Humidity & Temp Sensor

 Designed specifically to work with the AM2315 sensor from Adafruit
 ----> https://www.adafruit.com/products/1293

 These displays use I2C to communicate, 2 pins are required to
 interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries.

 Additional improvements added by Daniel Sandoval/EasternStarGeek
 8 July, 2013.  (Parent Library: Adafruit_AM2315)

 BSD license, all text above must be included in any redistribution
 ****************************************************/
#undef AM2315DEBUG
//#define AM2315DEBUG

#include "SDL_ESP8266_HR_AM2315.h"
//#include <util/delay.h>

SDL_ESP8266_HR_AM2315::SDL_ESP8266_HR_AM2315() {

  // intialize statistics
  goodReads = 0;
  badReads = 0;
  highBadReadCount = 0;
  badCRCReads = 0;
  badSpikeReads = 0;
  badMessageReads = 0;
  oldDataReads = 0;
  highTemp = -1000.0;
  lowTemp = 1000.0;
  lastTemp = 0.0;
  highBadReadCount = 0;
  for (int i = 0; i < 10; i++) {
    lastBadRead[i] = 0;
  }
  firstGoodRead = false;

  // Private.
  humidity = -1;
  temp =-1;
  badReadPresent = false;
  lastGoodTemp = -1;
  sentCRC = 0;
  calcCRC = 0;

#ifdef AM2315DEBUG
  randomSeed(analogRead(0));
#endif
}

// Constructors.
int delayByCPU(long delaycount);
uint16_t am2315_crc16(unsigned char *ptr, unsigned char len);

// Define read retur.n
#define GOODREAD 0
#define CRCERROR 1
#define BADTEMPERATURE 2
#define BADMESSAGE 3
#define OLDDATA 4

boolean SDL_ESP8266_HR_AM2315::readData(float *dataArray) {
  int returnValue;
  returnValue = internalReadData(dataArray);

#define REPEATCOUNT 7

  // OK now, we have a bad read, for any reason - repeat up to REPEATCOUNT times.

  if (returnValue != GOODREAD) {
    // This is for all error issues.
    int count;
    for (count = 0; count < REPEATCOUNT; count++) {
      returnValue = internalReadData(dataArray);

      switch (returnValue) {
        case GOODREAD: {
          goodReads++;
          if (count > highBadReadCount) {
            highBadReadCount = count;
          }

          // Now set high and low.
          if (dataArray[1] > highTemp) {
            highTemp = dataArray[1];
          }

          if (dataArray[1] < lowTemp) {
            lowTemp = dataArray[1];
          }
          return true;
        }
          break;

        case CRCERROR:
          badCRCReads++;
          badReads++;
          break;

        case BADTEMPERATURE:
          badSpikeReads++;
          badReads++;
          break;

        case BADMESSAGE:
          badMessageReads++;
          badReads++;
          break;

        default:
          break;

      } // End of switch.
      // We have fallen through loop - restore last good temperature and continue.
    } // End of For Loop.

    dataArray[1] = lastGoodTemp;
    oldDataReads++;

    return false;
  }  // End of Not GOODREAD clause.

  // Ah, good reads.
  goodReads++;
  highBadReadCount = 0;

  // Set high and low.
  if (dataArray[1] > highTemp) {
    highTemp = dataArray[1];
  }

  if (dataArray[1] < lowTemp) {
    lowTemp = dataArray[1];
  }

  return true;
}

int SDL_ESP8266_HR_AM2315::internalReadData(float * dataArray) {
  uint8_t reply[10];

  // ETS_UART_INTR_DISABLE();
  noInterrupts();
  ETS_INTR_LOCK();

  // Some ESP8266 devices require 5, 4 instead of 4,5.
  Wire.begin(5, 4);

  Wire.setClock(400000L);

  Wire.beginTransmission(AM2315_I2CADDR);
  Wire.write(AM2315_READREG);
  Wire.endTransmission();

  // delay(50);
  delayByCPU(50);

  Wire.beginTransmission(AM2315_I2CADDR);
  Wire.write(AM2315_READREG);
  // Start at address 0x0.
  Wire.write(0x00);
  // Request 4 bytes data.
  Wire.write(4);
  Wire.endTransmission();

  delayByCPU(50);

  Wire.requestFrom(AM2315_I2CADDR, 8);
  for (uint8_t i = 0; i < 8; i++) {
    reply[i] = Wire.read();
//    Serial.println(reply[i], HEX);
  }

  interrupts();
  ETS_UART_INTR_ENABLE();

  yield();

  calcCRC = am2315_crc16(reply, 6);

  sentCRC = (reply[7] << 8) + reply[6];

#ifdef AM2315DEBUG

  long randomNumber;

  // Make one out of 10 has a bad CRC error.
  randomNumber = random(0, 10);
  if (randomNumber == 0)
  _calcCRC++;

  Serial.print("calCRC=");
  Serial.println(_calcCRC, HEX);

  Serial.print("sentCRC=");
  Serial.println(_sentCRC, HEX);

#endif
  if (calcCRC != sentCRC) {
    // Bad CRC.
    badReadPresent = true;

    for (uint8_t i = 0; i < 8; i++) {
      lastBadRead[i] = reply[i];
    }

    dataArray[0] = NAN;
    dataArray[1] = NAN;

    return CRCERROR;
  }

  if ((reply[0] == AM2315_READREG) && (reply[1] == 4)) {
//    Serial.print("Humidity Read:");
//    Serial.print(" Reply2: ");
//    Serial.print(reply[2]);
//    Serial.print(" Reply3: ");
//    Serial.print(reply[3]);
    humidity = reply[2];
    humidity *= 256;
    humidity += reply[3];
    humidity /= 10;

    // Check for negative temperature.
    bool negative;
    negative = false;

    if (reply[4] & 0x80) {
      negative = true;
    }

    dataArray[0] = humidity;

    int temp = reply[4] & 0x7F;
    temp *= 256;
    temp += reply[5];
    temp /= 10;

    if (negative)
      temp = -temp;

#ifdef AM2315DEBUG

    long randomNumber;

    // Make one out of 10 has a bad Temperature Spike.
    randomNumber = random(0, 10);

    if (randomNumber == 0)
    temp = temp + 16.0;

#endif

    // Leave in C.
    // dataArray[1] = (temp * 1.8)+32;
    dataArray[1] = temp;

    // If we have a previous read, then check band.
    if (firstGoodRead == true) {

#define TEMPERATUREBAND 14.0

      if (lastGoodTemp > (TEMPERATUREBAND + temp)) {
        // Too high compared to last good value.
        for (uint8_t i = 0; i < 8; i++) {
          lastBadRead[i] = reply[i];
        }

        dataArray[0] = NAN;
        dataArray[1] = NAN;
        return BADTEMPERATURE;
      }
      if (lastGoodTemp < (temp - TEMPERATUREBAND)) {
        // Too low compared to last good value.
        for (uint8_t i = 0; i < 8; i++) {
          lastBadRead[i] = reply[i];
        }

        dataArray[0] = NAN;
        dataArray[1] = NAN;
        return BADTEMPERATURE;
      }
    }

    // Now we have passed the spike with good CRC problem, so we have goodness.
    firstGoodRead = true;
    lastGoodTemp = temp;

    return GOODREAD;
  } else {


    dataArray[0] = NAN;
    dataArray[1] = NAN;

    for (uint8_t i = 0; i < 8; i++) {
      lastBadRead[i] = reply[i];
    }

    return BADMESSAGE;
  }
}

/*
 Computes the crc code.

 @param char pointer
 @param length
 @return crc
 */
uint16_t am2315_crc16(unsigned char *ptr, unsigned char len) {
  unsigned short crc = 0xFFFF;
  unsigned char i;

  while (len--) {
    crc ^= *ptr++;
    for (i = 0; i < 8; i++) {
      if (crc & 0x01) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }

  }
  return crc;
}

int delayByCPU(long delaycount) {
  unsigned long startMillis;
  unsigned long endMillis;

#define COUNT 100000

  startMillis = millis();

  long index;
  float test;
  long i;
  long j;

  for (i = 0; i < delaycount * 166; i++) {
    // Serial.println("outside loop");
    for (j = 0; j < 1000; j++) {
      for (index = 0; index < COUNT; index++) {
        test = 30.4 + i;
        test = test / 50.0;
      }
      test = test + j;
    }
  }

  endMillis = millis();

  return int(test);
}

void SDL_ESP8266_HR_AM2315::printStatistics() {

  Serial.println("-----------------");
  Serial.println("AM2315 Statistics");
  Serial.println("-----------------");

  Serial.print("goodReads = ");
  Serial.println(goodReads);
  Serial.print("badReads = ");
  Serial.println(badReads);
  Serial.print("badCRCReads = ");
  Serial.println(badCRCReads);
  Serial.print("badSpikeReads = ");
  Serial.println(badSpikeReads);
  Serial.print("badMessageReads = ");
  Serial.println(badMessageReads);
  Serial.print("highTemp = ");
  Serial.println(highTemp);
  Serial.print("lowTemp = ");
  Serial.println(lowTemp);
  Serial.print("lastGoodTemp = ");
  Serial.println(lastGoodTemp);
  Serial.print("highSingleBadReadCount =");
  Serial.println(highBadReadCount);
  int i;
  Serial.print(":");
  for (i = 0; i < 10; i++) {
    Serial.print(lastBadRead[i], HEX);
    Serial.print(":");
  }
  Serial.println();

  Serial.println("-----------------");
}
