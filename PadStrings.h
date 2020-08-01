#ifndef __PadStrings__
#define __PadStrings__

void padByteToThreeDigits( uint8_t byteToPad, char* reference ) {
    
    reference[ 0 ] = '\0'; //make sure that reference passed is empty string
    
    char byteAsString[ 4 ] = "";
    snprintf( byteAsString, 4, "%d", byteToPad );
    
    if ( byteToPad > 99 ) {
        strcat( reference, byteAsString );
    } else if ( byteToPad > 9 ) {
        strcat( reference, "0" );
        strcat( reference, byteAsString );
    } else {
        strcat( reference, "00" );
        strcat( reference, byteAsString );
    }

} // end padByteToThreeDigits


void padByteToTwoDigits( int8_t byteToPad, char* reference ) {

    reference[ 0 ] = '\0'; //make sure that reference passed is empty string
    
    char byteAsString[ 3 ] = "";
    snprintf( byteAsString, 3, "%d", byteToPad );
    
    if ( byteToPad > 9 ) {
        strcat( reference, byteAsString );
    } else {
        strcat( reference, "0" );
        strcat( reference, byteAsString );
    }

} // end padByteToTwoDigits

#endif