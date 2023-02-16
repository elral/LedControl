/*
 *    LedControl.h - A library for controling Leds with a MAX7219/MAX7221
 *    Copyright (c) 2007 Eberhard Fahle
 *
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 *
 *    This permission notice shall be included in all copies or
 *    substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <Arduino.h>

/*
 * Segments to be switched on for characters and digits on
 * 7-Segment Displays
 * B01234567
 *
 *   1
 * 6   2
 *   7
 * 5   3
 *   4
 */
const static byte charTable[128] PROGMEM = {
    0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011, 0b01011111, 0b01110000,
    0b01111111, 0b01111011, 0b01110111, 0b00011111, 0b00001101, 0b00111101, 0b01001111, 0b01000111,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // (space)  ,!        ,"        ,#        ,$        ,%        ,&        ,'
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // (        ,)        ,*        ,+        ,,        ,-        ,.        ,/
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10000000, 0b00000001, 0b10000000, 0b00100101,
    // 0        ,1        ,2        ,3        ,4        ,5        ,6        ,7
    0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011, 0b01011111, 0b01110000,
    // 8        ,9        ,:        ,;        ,<        ,=        ,>        ,?
    0b01111111, 0b01111011, 0b00000000, 0b00000000, 0b00000000, 0b00001001, 0b00000000, 0b00000000,
    // @        ,A        ,0b        ,C        ,D        ,E        ,F        ,G
    0b00000000, 0b01110111, 0b01111111, 0b01001110, 0b01111110, 0b01001111, 0b01000111, 0b01011111,
    // H        ,I        ,J        ,K        ,L        ,M        ,N        ,O
    0b00110111, 0b00110000, 0b01111100, 0b00000000, 0b00001110, 0b00000000, 0b01110110, 0b01111110,
    // P        ,Q        ,R        ,S        ,T        ,U        ,V        ,W
    0b01100111, 0b01111110, 0b01110111, 0b01011011, 0b01000110, 0b00111110, 0b00000000, 0b00000000,
    // X        ,Y        ,Z        ,[        ,\        ,]        ,^        ,_
    0b00000000, 0b00110011, 0b01101101, 0b01001110, 0b00010011, 0b01111000, 0b00000000, 0b00001000,
    // `        ,a        ,0b        ,c        ,d        ,e        ,f        ,g
    0b00000000, 0b01111101, 0b00011111, 0b00001101, 0b00111101, 0b01001111, 0b01000111, 0b01111011,
    // h        ,i        ,j        ,k        ,l        ,m        ,n        ,o
    0b00110111, 0b00110000, 0b00111000, 0b00000000, 0b00000110, 0b00000000, 0b00010101, 0b00011101,
    // p        ,q        ,r        ,s        ,t        ,u        ,v        ,w
    0b01100111, 0b01110011, 0b00000101, 0b01011011, 0b00001111, 0b00011100, 0b00000000, 0b00000000,
    // x        ,y        ,z        ,{        ,|        ,}        ,~        ,(delete)
    0b00000000, 0b00100111, 0b01101101, 0b01001110, 0b00000000, 0b01111000, 0b00000000, 0b00000000};

class LedControl
{
#ifdef USE_FAST_IO

#define DIGITALREAD(a)     digitalReadFast(a)
#define DIGITALWRITE(a, b) digitalWriteFast(a, b)
    typedef struct {
#ifdef ARDUINO_ARCH_RP2040
        volatile uint32_t *Port;
        uint32_t           Mask;
#else
        volatile uint8_t *Port;
        uint8_t           Mask;
#endif
    } FASTIO_s;

    inline void digitalWriteFast(FASTIO_s Pin, uint8_t value)
    {
        if (value)
            *Pin.Port |= Pin.Mask;
        else
            *Pin.Port &= ~Pin.Mask;
    }

    inline uint8_t digitalReadFast(FASTIO_s Pin)
    {
        if (*Pin.Port & Pin.Mask) return HIGH;
        return LOW;
    }

#else

#define DIGITALREAD(a)     digitalRead(a)
#define DIGITALWRITE(a, b) digitalWrite(a, b)

    typedef uint8_t FASTIO_s;

#endif

private:
    /* The array for shifting the data to the devices */
    byte spidata[16];
    /* Send out a single command to the device */
    void spiTransfer(int addr, byte opcode, byte data);
#ifndef MF_REDUCE_FUNCT_LEDCONTROL
    /* We keep track of the led-status for all 8 devices in this array */
    byte status[64];
#endif
    /* Data is shifted out of this pin*/
    FASTIO_s SPI_MOSI;
    /* The clock is signaled on this pin */
    FASTIO_s SPI_CLK;
    /* This one is driven LOW for chip selectzion */
    FASTIO_s SPI_CS;
    /* The maximum number of devices we use */
    int maxDevices;

public:
    /*
     * Create a new controler
     * Params :
     */
    LedControl();

    /*
     * Defines the new controler
     * Params :
     * dataPin		pin on the Arduino where data gets shifted out
     * clockPin		pin for the clock
     * csPin		pin for selecting the device
     * numDevices	maximum number of devices that can be controled
     */
    void begin(int dataPin, int clkPin, int csPin, int numDevices = 1);

    /*
     * Gets the number of devices attached to this LedControl.
     * Returns :
     * int	the number of devices on this LedControl
     */
    int getDeviceCount();

    /*
     * Set the shutdown (power saving) mode for the device
     * Params :
     * addr	The address of the display to control
     * status	If true the device goes into power-down mode. Set to false
     *		for normal operation.
     */
    void shutdown(int addr, bool status);

    /*
     * Set the number of digits (or rows) to be displayed.
     * See datasheet for sideeffects of the scanlimit on the brightness
     * of the display.
     * Params :
     * addr	address of the display to control
     * limit	number of digits to be displayed (1..8)
     */
    void setScanLimit(int addr, int limit);

    /*
     * Set the brightness of the display.
     * Params:
     * addr		the address of the display to control
     * intensity	the brightness of the display. (0..15)
     */
    void setIntensity(int addr, int intensity);

    /*
     * Switch all Leds on the display off.
     * Params:
     * addr	address of the display to control
     */
    void clearDisplay(int addr);

#ifndef MF_REDUCE_FUNCT_LEDCONTROL
    /*
     * Set the status of a single Led.
     * Params :
     * addr	address of the display
     * row	the row of the Led (0..7)
     * col	the column of the Led (0..7)
     * state	If true the led is switched on,
     *		if false it is switched off
     */
    void setLed(int addr, int row, int col, boolean state);

    /*
     * Set all 8 Led's in a row to a new state
     * Params:
     * addr	address of the display
     * row	row which is to be set (0..7)
     * value	each bit set to 1 will light up the
     *		corresponding Led.
     */
    void setRow(int addr, int row, byte value);

    /*
     * Set all 8 Led's in a column to a new state
     * Params:
     * addr	address of the display
     * col	column which is to be set (0..7)
     * value	each bit set to 1 will light up the
     *		corresponding Led.
     */
    void setColumn(int addr, int col, byte value);
#endif
    /*
     * Display a hexadecimal digit on a 7-Segment Display
     * Params:
     * addr	address of the display
     * digit	the position of the digit on the display (0..7)
     * value	the value to be displayed. (0x00..0x0F)
     * dp	sets the decimal point.
     */
    void setDigit(int addr, int digit, byte value, boolean dp);

    /*
     * Display a character on a 7-Segment display.
     * There are only a few characters that make sense here :
     *	'0','1','2','3','4','5','6','7','8','9','0',
     *  'A','b','c','d','E','F','H','L','P',
     *  '.','-','_',' '
     * Params:
     * addr	address of the display
     * digit	the position of the character on the display (0..7)
     * value	the character to be displayed.
     * dp	sets the decimal point.
     */
    void setChar(int addr, int digit, char value, boolean dp);
};
