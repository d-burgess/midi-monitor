// information from https://tttapa.github.io/PDF/Arduino-MIDI.pdf

// #include <LiquidCrystal.h> // Import the LCD library
#include <LiquidCrystalFast.h>
// LiquidCrystal lcd( 2, 3, 4, 5, 6, 7 ); // Initialize the LCD
LiquidCrystalFast lcd(2, 8, 3, 4, 5, 6, 7);
         // LCD pins: RS RW E  D4 D5 D6 D7

#include "FIFO.h" // fifo buffer

FIFO serialByteBuffer;

void setup() {
  Serial.begin( 31250 );
  while ( !Serial ) delay( 1 ); // wait until the serial port has opened
  delay( 100 );
  lcd.begin( 16, 2 ); // tell the LCD that it is a 16x2 LCD
  lcd.setCursor( 0, 0 );
  lcd.clear();
  lcd.print( "MIDIMON..." );
  lcd.setCursor( 11, 0 );
  lcd.print( "CH:" );
}

const uint8_t NOTE_OFF = 0x8;
const uint8_t NOTE_ON = 0x9;
const uint8_t KEY_PRESSURE = 0xA;
const uint8_t CC = 0xB;
const uint8_t PROGRAM_CHANGE = 0xC;
const uint8_t CHANNEL_PRESSURE = 0xD;
const uint8_t PITCH_BEND = 0xE;
const uint8_t STATUS_BIT = 0b10000000;
const uint8_t DATA_UNSTORED = 0xFF;

void reportMIDI( uint8_t statusByte, uint8_t dataByte2, uint8_t dataByte3 = 0 ) {
  
  if ( serialByteBuffer.size() == 0 ) {
    uint8_t messageTypeNibble = statusByte >> 4 & 0b00001111;
    uint8_t channelNibble = statusByte & 0b00001111;
    String channel = String( channelNibble+1 );
    
    String messageType;
    String dataType2;
    String dataType3;
    
    switch ( messageTypeNibble ) {
      case NOTE_OFF:
        messageType = "NOTE OFF  ";
        dataType2 = "PCH:";
        dataType3 = "VEL:";
        break;
      case NOTE_ON:
        messageType = "NOTE ON   ";
        dataType2 = "PCH:";
        dataType3 = "VEL:";
        break;
      case KEY_PRESSURE:
        messageType = "KEY PRESSR";
        dataType2 = "PCH:";
        dataType3 = "PRS:";
        break;
      case CC:
        messageType = "CNTRL CHNG";
        dataType2 = "NUM:";
        dataType3 = "VAL:";
        break;
      case PROGRAM_CHANGE:
        messageType = "PRGRM CHNG";
        dataType2 = "PGM:";
        dataType3 = "    ";
        break;
      case CHANNEL_PRESSURE:
        messageType = "CHN PRESSR";
        dataType2 = "PRS:";
        dataType3 = "    ";
        break;
      case PITCH_BEND:
        messageType = "PITCH BEND";
        dataType2 = "LSB:";
        dataType3 = "MSB:";
        break;
      default:
        messageType = "UNKNOWN?  ";
        dataType2 = "    ";
        dataType3 = "    ";
        break;
    }
  
    // Row 0
    lcd.setCursor( 0, 0 );
    lcd.print( messageType );
    
    lcd.setCursor( 14, 0 );
    if ( channelNibble > 8 ) {
      lcd.print( channel );
    } else { 
      lcd.print( "0" + channel );
    }
    
    // Row 1
    lcd.setCursor( 0, 1 );
    lcd.print( dataType2 );
    lcd.setCursor( 4, 1 );
    if ( dataByte2 > 99 ) {
      lcd.print( String( dataByte2 ) );
    } else if ( dataByte2 > 9 ) {
      lcd.print( "0" + String( dataByte2 ) );
    } else {
      lcd.print( "00" + String( dataByte2 ) );
    }
    
    lcd.setCursor( 9, 1 );
    if ( dataByte3 != DATA_UNSTORED ) {
      lcd.print( dataType3 );
      lcd.setCursor( 13, 1 );
      if ( dataByte3 > 99 ) {
        lcd.print( String( dataByte3 ) );
      } else if ( dataByte3 > 9 ) {
        lcd.print( "0" + String( dataByte3 ) );
      } else {
        lcd.print( "00" + String( dataByte3 ) );
      }
    } else {
      lcd.print( "       " );
    }
  } // end serial byte buffer empty
} // end reportMidi

void loop() {

  static uint8_t serialRunningStatus; // data bytes may be repeated without status bytes
  static bool serialThirdByteFlag = false;
  
  // read a byte from serial and add to buffer
  if ( Serial.available() ) {
    uint8_t serialByte = Serial.read();
  
    if ( serialByte & 0b10000000 ) { // Status byte received
      serialRunningStatus = serialByte;
      serialThirdByteFlag = false;
    } else {
      if ( serialThirdByteFlag ) { // Second data byte received
        serialThirdByteFlag = false;
        serialByteBuffer.push( serialByte );
        return;
      } else { // First data byte received
        if ( !serialRunningStatus ) { // no status byte
          return; // ignore invalid data byte
        } else {
          // buffer greater than 0
          if ( serialRunningStatus < 0xC0 ) { // First data byte of Note Off/On, Key Pressure or Control Change
            serialThirdByteFlag = true;
            serialByteBuffer.push( serialRunningStatus );
            serialByteBuffer.push( serialByte );
            return;
          }
          if ( serialRunningStatus < 0xE0 ) { // First data byte of Program Change or Channel Pressure
            serialByteBuffer.push( serialRunningStatus );
            serialByteBuffer.push( serialByte );
            return;
          }
          if ( serialRunningStatus < 0xF0 ) { // First data byte of Pitch Bend
            serialThirdByteFlag = true;
            serialByteBuffer.push( serialRunningStatus );
            serialByteBuffer.push( serialByte );
            return;
          } else { // System message
            serialRunningStatus = 0;
            return;
          }
        } // end running status buffer not empty 
      } // end not third data byte
    } // end not header byte
  } // end serial available

  if ( serialByteBuffer.size() > 0 ) {
    static uint8_t bufferRunningStatus = 0;
    static uint8_t bufferDataByte2 = DATA_UNSTORED;
    static bool bufferThirdByteFlag = false;
    
    uint8_t bufferByte = serialByteBuffer.pop();
    Serial.write(bufferByte);
    
    if ( bufferByte & 0b10000000 ) { // Status byte received
      bufferRunningStatus = bufferByte;
      bufferThirdByteFlag = false;
    } else {
      if ( bufferThirdByteFlag ) { // Second data byte received
        bufferThirdByteFlag = false;
        reportMIDI( bufferRunningStatus, bufferDataByte2, bufferByte );
        bufferDataByte2 = DATA_UNSTORED;
        return;
      } else { // First data byte received
        if ( !bufferRunningStatus ) { // no status byte
          return; // ignore invalid data byte
        } else {
          if ( bufferRunningStatus < 0xC0 ) { // Note Off/On, Key Pressure or Control Change
            bufferThirdByteFlag = true;
            bufferDataByte2 = bufferByte;
            //
            return;
          }
          if ( bufferRunningStatus < 0xE0 ) { // Program Change or Channel Pressure
            reportMIDI( bufferRunningStatus, bufferByte, DATA_UNSTORED );
            //
            return;
          }
          if ( bufferRunningStatus < 0xF0 ) { // Pitch Bend
            bufferThirdByteFlag = true;
            bufferDataByte2 = bufferByte;
            //
            return;
          } else { // System message
            bufferRunningStatus = 0;
            return;
          }
        } // end running status buffer not empty 
      } // end not third data byte
    } // end not header byte
  } // end buffer not empty
  
} // end loop
