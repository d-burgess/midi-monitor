// MIDI Monitor for Teensy 3.2/Arduino Uno

#include "MidiMonitor.h"

void setup() {
  
  HWSERIAL.begin( 31250 );
  while ( !HWSERIAL ) delay( 1 ); // wait until the serial port has opened

  initialiseLcd();
  updateLcdPendingBuffer( "MIDI MONITOR ...", 0 );
  updateLcdPendingBuffer( BOARD, 1 );
  updateLcd();

}


void loop() {

  unsigned long currentMicros = micros();

  static uint8_t serialRunningStatus; // data bytes may be repeated without status bytes
  static bool serialThirdByteFlag = false;
  
  // read a byte from serial and add to serial byte buffer
  if ( HWSERIAL.available() > 0 ) {
    uint8_t serialByte = HWSERIAL.read();
    // Serial.print( String (serialByte, BIN ) );
    HWSERIAL.write( serialByte );
    previousMicros = currentMicros;
    if ( serialByte & 0b10000000 ) { // Status byte received
      serialRunningStatus = serialByte;
      serialThirdByteFlag = false;
      // Serial.println( " STATUS " );
    } else {
      if ( serialThirdByteFlag ) { // Second data byte received
        serialThirdByteFlag = false;
        serialByteBuffer.push( serialByte );
        // Serial.println( " DATA 3 " );
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
            // Serial.println( " DATA 2 " );
            return;
          }
          if ( serialRunningStatus < 0xE0 ) { // First data byte of Program Change or Channel Pressure
            serialByteBuffer.push( serialRunningStatus );
            serialByteBuffer.push( serialByte );
            // Serial.println( " DATA 2 " );
            return;
          }
          if ( serialRunningStatus < 0xF0 ) { // First data byte of Pitch Bend
            serialThirdByteFlag = true;
            serialByteBuffer.push( serialRunningStatus );
            serialByteBuffer.push( serialByte );
            // Serial.println( " DATA 2 " );
            return;
          } else { // System message
            serialRunningStatus = 0;
            // Serial.println( " SYSTEM " );
            return;
          }
        } // end running status buffer not empty 
      } // end not third data byte
    } // end not header byte
  } // end serial available

  // read a byte from the serial byte buffer, 
  // write byte to output 
  // and report if whole MIDI message is received
  if ( HWSERIAL.available() == 0 ) {
    if ( serialByteBuffer.size() > 0 ) {
      // Serial.print( "Buffer size  " );
      // Serial.println( serialByteBuffer.size() );
      if ( serialByteBuffer.size() >= ( FIFO_SIZE - 1) ) {
        lcd.setCursor ( 0, 0 );
        lcd.print( "BUFFER OVERFLOW!" );
        exit( 0 );
      }
      if ( currentMicros - previousMicros >= refreshRate ) {
        // Serial.print( "Refresh delay " );
        // Serial.println( currentMicros - previousMicros );
        previousMicros = currentMicros;
        
        static uint8_t bufferRunningStatus = 0;
        static uint8_t bufferDataByte2 = DATA_UNSTORED;
        static bool bufferThirdByteFlag = false;
        
        uint8_t bufferByte = serialByteBuffer.pop();
        // Serial.write(bufferByte);
        
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
      } // end refresh rate reached
    } // end buffer not empty
  }
  
} // end loop
