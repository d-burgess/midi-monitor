#ifndef __LcdBuffer__
#define __LcdBuffer__

#include "src/LiquidCrystalFast/LiquidCrystalFast.h" // Import the LCD Fast library

LiquidCrystalFast lcd(2, 8, 3, 4, 5, 6, 7); // Initialize the LCD
         // LCD pins: RS RW E  D4 D5 D6 D7  // Uses 7 pins, RW not to GND

#define LCD_SIZE 16

const char BLANK_ROW[ LCD_SIZE + 1 ] = "                ";

// what should be currently on the display of the lcd
char lcdRow0[ LCD_SIZE + 1 ] = "";
char lcdRow1[ LCD_SIZE + 1 ] = "";

// buffers that hold what is next on the display of the lcd
char lcdPendingRow0Buffer[ LCD_SIZE + 1 ] = "";
char lcdPendingRow1Buffer[ LCD_SIZE + 1 ] = "";

// initialise lcd
void initialiseLcd() {

  lcd.begin( LCD_SIZE, 2 ); // tell the LCD that it is a 16x2 LCD
  lcd.setCursor( 0, 0 );
  lcd.clear();
  
} // end initialiseLcd

// update the whole lcd
void updateLcd() {

  size_t i = 0;
  
  // update row 0 with changed characters
  for ( i = 0; i < strlen( lcdPendingRow0Buffer ); i++ ) {
    if ( lcdPendingRow0Buffer[ i ] != lcdRow0[ i ] ) {
      // char element of pending buffer does not match char element of lcd, update
      lcd.setCursor ( i, 0 );
      lcd.print( lcdPendingRow0Buffer[ i ] );
      memcpy( lcdRow0 + i, lcdPendingRow0Buffer + i, 1 );
    }
  }
  
  // update row 1 with changed characters
  for ( i = 0; i < strlen( lcdPendingRow1Buffer ); i++ ) {
    if ( lcdPendingRow1Buffer[ i ] != lcdRow1[ i ] ) {
      // char element of pending buffer does not match char element of lcd, update
      lcd.setCursor ( i, 1 );
      lcd.print( lcdPendingRow1Buffer[ i ] );
      memcpy( lcdRow1 + i, lcdPendingRow1Buffer + i, 1 );
    }
  }
  
} // end updateLcd

// set the lcd pending buffer
void updateLcdPendingBuffer( const char message[] = BLANK_ROW, uint8_t row = 0 ) {

  char output[ LCD_SIZE + 1 ];

  if ( strlen( message ) < LCD_SIZE ) {
    // pad with spaces
    sprintf( output, "%-16s", message ); // need LCD_SIZE here
  } else if ( strlen( message ) > LCD_SIZE ) {
    // limit to LCD_SIZE
    memcpy( output, &message[0], LCD_SIZE );
    output[ LCD_SIZE ] = '\0';
  } else {
    strcpy( output, message );
  }

  // update buffer
  if ( row == 1 ) {
    strcpy( lcdPendingRow1Buffer, output );
  } else {
    strcpy( lcdPendingRow0Buffer, output );
  }
  
} // end updateLcdPendingBuffer

#endif
