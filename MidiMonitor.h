#ifndef __MidiMonitor__
#define __MidiMonitor__

#include "LcdBuffer.h"

#include "PadStrings.h"

#include "src/FIFO/FIFO.h" // Import FIFO buffer
FIFO serialByteBuffer; // edit buffer size in FIFO.h

// Set serial port
#if defined( TEENSYDUINO ) 
  #if defined( __MK20DX256__ )       
    #define BOARD "Teensy 3.2"
    #define HWSERIAL Serial1
  #endif
#elif defined( ARDUINO_AVR_UNO )       
  #define BOARD "Arduino Uno"
  #define HWSERIAL Serial
#else
  #define BOARD "Unknown"
  #define HWSERIAL Serial
#endif

const uint8_t NOTE_OFF = 0x8;
const uint8_t NOTE_ON = 0x9;
const uint8_t KEY_PRESSURE = 0xA;
const uint8_t CONTROL_CHANGE = 0xB;
const uint8_t PROGRAM_CHANGE = 0xC;
const uint8_t CHANNEL_PRESSURE = 0xD;
const uint8_t PITCH_BEND = 0xE;
const uint8_t STATUS_BIT = 0b10000000;
const uint8_t DATA_UNSTORED = 0xFF;

unsigned long previousMicros = 0;
unsigned long refreshRate = 75; // microseconds

void reportMIDI( uint8_t statusByte, uint8_t dataByte2, uint8_t dataByte3 = 0 ) {
  
  if ( serialByteBuffer.size() == 0 ) {
    uint8_t messageTypeNibble = statusByte >> 4 & 0b00001111;
    uint8_t channelNibble = statusByte & 0b00001111;
    uint8_t channel = channelNibble + 1;
    
    char messageType[ 11 ] = "";
    char dataType2[ 5 ] = "";
    char dataType3[ 5  ] = "";
    char row0[ LCD_SIZE + 1 ] = "";
    char row1[ LCD_SIZE + 1 ] = "";
    
    switch ( messageTypeNibble ) {
      case NOTE_OFF:
        strcpy( messageType, "NOTE OFF  " );
        strcpy( dataType2, "PCH:" );
        strcpy( dataType3, "VEL:" );
        break;
      case NOTE_ON:
        strcpy( messageType, "NOTE ON   " );
        strcpy( dataType2, "PCH:" );
        strcpy( dataType3, "VEL:" );
        break;
      case KEY_PRESSURE:
        strcpy( messageType, "KEY PRESSR" );
        strcpy( dataType2, "PCH:" );
        strcpy( dataType3, "PRS:" );
        break;
      case CONTROL_CHANGE:
        strcpy( messageType, "CNTRL CHNG" );
        strcpy( dataType2, "NUM:" );
        strcpy( dataType3, "VAL:" );
        break;
      case PROGRAM_CHANGE:
        strcpy( messageType, "PRGRM CHNG" );
        strcpy( dataType2, "PGM:" );
        strcpy( dataType3, "    " );
        break;
      case CHANNEL_PRESSURE:
        strcpy( messageType, "CHN PRESSR" );
        strcpy( dataType2, "PRS:" );
        strcpy( dataType3, "    " );
        break;
      case PITCH_BEND:
        strcpy( messageType, "PITCH BEND" );
        strcpy( dataType2, "LSB:" );
        strcpy( dataType3, "MSB:" );
        break;
      default:
        strcpy( messageType, "UNKNOWN?  " );
        strcpy( dataType2, "    " );
        strcpy( dataType3, "    " );
        break;
    }
  
 
    strcat( row0, messageType );
    strcat( row0, " CH:" );
    char paddedChannel[ 3 ];
    padByteToTwoDigits( channel, paddedChannel );
    strcat( row0, paddedChannel );

    strcat( row1, dataType2 );
    char paddedDataByte2[ 4 ];
    padByteToThreeDigits( dataByte2, paddedDataByte2 );
    strcat( row1, paddedDataByte2 );
    strcat( row1, " " );

    if ( dataByte3 != DATA_UNSTORED ) {
      strcat( row1, dataType3 );
      char paddedDataByte3[ 4 ];
      padByteToThreeDigits( dataByte3, paddedDataByte3 );
      strcat( row1, paddedDataByte3 );
      strcat( row1, " " );
    } else {
      strcat ( row1, "        " );
    }

    updateLcdPendingBuffer( row0, 0 );
    updateLcdPendingBuffer( row1, 1 );

    updateLcd();

  } // end serial byte buffer empty
} // end reportMidi

#endif
