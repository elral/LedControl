/*
 *    LedControl.cpp - A library for controling Leds with a MAX7219/MAX7221
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

#include "LedControl.h"

// the opcodes for the MAX7221 and MAX7219
#define OP_NOOP        0
#define OP_DIGIT0      1
#define OP_DIGIT1      2
#define OP_DIGIT2      3
#define OP_DIGIT3      4
#define OP_DIGIT4      5
#define OP_DIGIT5      6
#define OP_DIGIT6      7
#define OP_DIGIT7      8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

LedControl::LedControl() {}

void LedControl::begin(int dataPin, int clkPin, int csPin, int numDevices)
{

#ifdef USE_FAST_IO
    SPI_MOSI.Port = portOutputRegister(digitalPinToPort(dataPin));
    SPI_MOSI.Mask = digitalPinToBitMask(dataPin);
    SPI_CLK.Port  = portOutputRegister(digitalPinToPort(clkPin));
    SPI_CLK.Mask  = digitalPinToBitMask(clkPin);
    SPI_CS.Port   = portOutputRegister(digitalPinToPort(csPin));
    SPI_CS.Mask   = digitalPinToBitMask(csPin);
#else
    SPI_MOSI = dataPin;
    SPI_CLK  = clkPin;
    SPI_CS   = csPin;
#endif
    if (numDevices <= 0 || numDevices > 8)
        numDevices = 8;
    maxDevices = numDevices;
    pinMode(dataPin, OUTPUT);
    pinMode(clkPin, OUTPUT);
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
#ifndef MF_REDUCE_FUNCT_LEDCONTROL
    for (int i = 0; i < 64; i++)
        status[i] = 0x00;
#endif
    for (int i = 0; i < maxDevices; i++) {
        spiTransfer(i, OP_DISPLAYTEST, 0);
        // scanlimit is set to max on startup
        setScanLimit(i, 7);
        // decode is done in source
        spiTransfer(i, OP_DECODEMODE, 0);
        clearDisplay(i);
        // we go into shutdown-mode on startup
        shutdown(i, true);
    }
}

void LedControl::shutdown(int addr, bool b)
{
    if (addr < 0 || addr >= maxDevices)
        return;
    if (b)
        spiTransfer(addr, OP_SHUTDOWN, 0);
    else
        spiTransfer(addr, OP_SHUTDOWN, 1);
}

void LedControl::setScanLimit(int addr, int limit)
{
    if (addr < 0 || addr >= maxDevices)
        return;
    if (limit >= 0 || limit < 8)
        spiTransfer(addr, OP_SCANLIMIT, limit);
}

void LedControl::setIntensity(int addr, int intensity)
{
    if (addr < 0 || addr >= maxDevices)
        return;
    if (intensity >= 0 || intensity < 16)
        spiTransfer(addr, OP_INTENSITY, intensity);
}

void LedControl::clearDisplay(int addr)
{
    if (addr < 0 || addr >= maxDevices)
        return;
#ifndef MF_REDUCE_FUNCT_LEDCONTROL
    int offset = addr * 8;
#endif
    for (int i = 0; i < 8; i++) {
#ifndef MF_REDUCE_FUNCT_LEDCONTROL
        status[offset + i] = 0;
#endif
        spiTransfer(addr, i + 1, 0);
    }
}
#ifndef MF_REDUCE_FUNCT_LEDCONTROL
int LedControl::getDeviceCount()
{
    return maxDevices;
}

void LedControl::setLed(int addr, int row, int column, boolean state)
{
    int  offset;
    byte val = 0x00;

    if (addr < 0 || addr >= maxDevices)
        return;
    if (row < 0 || row > 7 || column < 0 || column > 7)
        return;
    offset = addr * 8;
    val    = 0b10000000 >> column;
    if (state)
        status[offset + row] = status[offset + row] | val;
    else {
        val                  = ~val;
        status[offset + row] = status[offset + row] & val;
    }
    spiTransfer(addr, row + 1, status[offset + row]);
}

void LedControl::setRow(int addr, int row, byte value)
{
    int offset;
    if (addr < 0 || addr >= maxDevices)
        return;
    if (row < 0 || row > 7)
        return;
    offset               = addr * 8;
    status[offset + row] = value;
    spiTransfer(addr, row + 1, status[offset + row]);
}

void LedControl::setColumn(int addr, int col, byte value)
{
    byte val;

    if (addr < 0 || addr >= maxDevices)
        return;
    if (col < 0 || col > 7)
        return;
    for (int row = 0; row < 8; row++) {
        val = value >> (7 - row);
        val = val & 0x01;
        setLed(addr, row, col, val);
    }
}
#endif

void LedControl::setDigit(int addr, int digit, byte value, boolean dp)
{
    if (addr < 0 || addr >= maxDevices)
        return;
    if (digit < 0 || digit > 7 || value > 15)
        return;
#ifndef MF_REDUCE_FUNCT_LEDCONTROL
    int offset = addr * 8;
#endif
    byte v;
    v = pgm_read_byte_near(charTable + value);
    if (dp)
        v |= 0b10000000;
#ifndef MF_REDUCE_FUNCT_LEDCONTROL
    status[offset + digit] = v;
#endif
    spiTransfer(addr, digit + 1, v);
}

void LedControl::setChar(int addr, int digit, char value, boolean dp)
{
    if (addr < 0 || addr >= maxDevices)
        return;
    if (digit < 0 || digit > 7)
        return;
#ifndef MF_REDUCE_FUNCT_LEDCONTROL
    int offset = addr * 8;
#endif
    byte index, v;

    index = (byte)value;
    if (index > 127) {
        // nothing define we use the space char
        value = 32;
    }
    v = pgm_read_byte_near(charTable + index);
    if (dp)
        v |= 0b10000000;
#ifndef MF_REDUCE_FUNCT_LEDCONTROL
    status[offset + digit] = v;
#endif
    spiTransfer(addr, digit + 1, v);
}

void LedControl::spiTransfer(int addr, volatile byte opcode, volatile byte data)
{
    // Create an array with the data to shift out
    int offset   = addr * 2;
    int maxbytes = maxDevices * 2;

    for (int i = 0; i < maxbytes; i++)
        spidata[i] = (byte)0;
    // put our device data into the array
    spidata[offset + 1] = opcode;
    spidata[offset]     = data;
    // enable the line
    DIGITALWRITE(SPI_CS, LOW);
    // Now shift out the data
    for (int i = maxbytes; i > 0; i--) {
        for (int8_t j = 7; j >= 0; j--) {
            DIGITALWRITE(SPI_MOSI, (spidata[i - 1] & (1 << (j))));
            DIGITALWRITE(SPI_CLK, HIGH);
            DIGITALWRITE(SPI_CLK, LOW);
        }
    }
    // latch the data onto the display
    DIGITALWRITE(SPI_CS, HIGH);
}
