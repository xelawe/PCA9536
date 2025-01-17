/*==============================================================================================================*

    @file     PCA9536.cpp
    @author   Nadav Matalon
    @license  MIT (c) 2016 Nadav Matalon

    PCA9536 Driver (4-Channel GPIO I2C Expander)

    Ver. 1.0.0 - First release (24.10.16)
 
 *===============================================================================================================*
    LICENSE
 *===============================================================================================================*
 
    The MIT License (MIT)
    Copyright (c) 2016 Nadav Matalon

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
    documentation files (the "Software"), to deal in the Software without restriction, including without
    limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial
    portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
    LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 *==============================================================================================================*/

#if 1
__asm volatile ("nop");
#endif

#include "PCA9536.h"

/*==============================================================================================================*
    CONSTRUCTOR
 *==============================================================================================================*/

PCA9536::PCA9536() {
    _comBuffer = COM_SUCCESS;
}

/*==============================================================================================================*
    DESTRUCTOR
 *==============================================================================================================*/

PCA9536::~PCA9536() {}

/*==============================================================================================================*
    PING (0 = SUCCESS / 1, 2... = ERROR CODE)
 *==============================================================================================================*/

// For meaning of I2C Error Codes see README

byte PCA9536::ping() {
    Wire.beginTransmission(DEV_ADDR);
    return Wire.endTransmission();
}

/*==============================================================================================================*
    GET MODE (0 = OUTPUT / 1 = INPUT)
 *==============================================================================================================*/

byte PCA9536::getMode(pin_t pin) {
    return getPin(pin, REG_CONFIG);
}

/*==============================================================================================================*
    GET STATE (0 = LOW / 1 = HIGH)
 *==============================================================================================================*/

byte PCA9536::getState(pin_t pin) {
    return getPin(pin, getMode(pin) ? REG_INPUT : REG_OUTPUT);
}

/*==============================================================================================================*
    GET POLARITY: INPUT PINS ONLY (0 = NON-INVERTED / 1 = INVERTED)
 *==============================================================================================================*/

byte PCA9536::getPolarity(pin_t pin) {
    return getPin(pin, REG_POLARITY);
}

/*==============================================================================================================*
    SET MODE
 *==============================================================================================================*/

void PCA9536::setMode(pin_t pin, mode_t newMode) {                           // PARAMS: IO0 / IO1 / IO2 / IO3
    setPin(pin, REG_CONFIG, newMode);                                        //         IO_INPUT / IO_OUTPUT
}

/*==============================================================================================================*
    SET MODE : ALL PINS
 *==============================================================================================================*/

void PCA9536::setMode(mode_t newMode) {                                      // PARAMS: IO_INPUT / IO_OUTPUT
    setReg(REG_CONFIG, newMode ? ALL_INPUT : ALL_OUTPUT);
}

/*==============================================================================================================*
    SET STATE (OUTPUT PINS ONLY)
 *==============================================================================================================*/

void PCA9536::setState(pin_t pin, state_t newState) {                        // PARAMS: IO0 / IO1 / IO2 / IO3
    setPin(pin, REG_OUTPUT, newState);                                       //         IO_LOW / IO_HIGH
}

/*==============================================================================================================*
    SET STATE : ALL PINS (OUTPUT PINS ONLY)
 *==============================================================================================================*/

void PCA9536::setState(state_t newState) {                                   // PARAMS: IO_LOW / IO_HIGH
    setReg(REG_OUTPUT, newState ? ALL_HIGH : ALL_LOW);
}

/*==============================================================================================================*
    SET STATE : ALL PINS with pattern (OUTPUT PINS ONLY)
 *==============================================================================================================*/

void PCA9536::setState(state_t newState0, state_t newState1, state_t newState2, state_t newState3) {  // PARAMS: 
    setPins(REG_OUTPUT, newState0, newState1, newState2, newState3);       //         IO_LOW / IO_HIGH for I0 to I3
}

/*==============================================================================================================*
    TOGGLE STATE (OUTPUT PINS ONLY)
 *==============================================================================================================*/

void PCA9536::toggleState(pin_t pin) {
    setReg(REG_OUTPUT, getReg(REG_OUTPUT) ^ (1 << pin));
}

/*==============================================================================================================*
    TOGGLE STATE : ALL PINS (OUTPUT PINS ONLY)
 *==============================================================================================================*/

void PCA9536::toggleState() {
    setReg(REG_OUTPUT, ~getReg(REG_OUTPUT));
}

/*==============================================================================================================*
    SET POLARITY (INPUT PINS ONLY)
 *==============================================================================================================*/

void PCA9536::setPolarity(pin_t pin, polarity_t newPolarity) {          // PARAMS: IO0 / IO1 / IO2 / IO3
    setPin(pin, REG_POLARITY, newPolarity);                             //         IO_NON_INVERTED / IO_INVERTED
}

/*==============================================================================================================*
    SET POLARITY : ALL PINS (INPUT PINS ONLY)
 *==============================================================================================================*/

void PCA9536::setPolarity(polarity_t newPolarity) {                     // PARAMS: IO_NON_INVERTED / IO_INVERTED
    byte polarityVals, polarityMask, polarityNew;
    polarityVals = getReg(REG_POLARITY);
    polarityMask = getReg(REG_CONFIG);
    polarityNew  = newPolarity ? ALL_INVERTED : ALL_NON_INVERTED;
    setReg(REG_POLARITY, (polarityVals & ~polarityMask) | (polarityNew & polarityMask));
}

/*==============================================================================================================*
    RESET
 *==============================================================================================================*/

void PCA9536::reset() {
    setMode(IO_INPUT);
    setState(IO_HIGH);
    setPolarity(IO_NON_INVERTED);
    initCall(REG_INPUT);
    endCall();
}

/*==============================================================================================================*
    GET REGISTER DATA
 *==============================================================================================================*/

byte PCA9536::getReg(reg_ptr_t regPtr) {
    byte regData = 0;
    initCall(regPtr);
    endCall();
    if (_comBuffer == COM_SUCCESS) {
        Wire.requestFrom(DEV_ADDR, NUM_BYTES);
        if (Wire.available() == NUM_BYTES) regData = Wire.read();
        else {
            while (Wire.available()) Wire.read();
            _comBuffer = ping();
        }
    }
    return regData;
}

/*==============================================================================================================*
    GET PIN DATA
 *==============================================================================================================*/

byte PCA9536::getPin(pin_t pin, reg_ptr_t regPtr) {
    return bitRead(getReg(regPtr), pin);
}

/*==============================================================================================================*
    SET REGISTER DATA
 *==============================================================================================================*/

void PCA9536::setReg(reg_ptr_t regPtr, byte newSetting) {
    if (regPtr > 0) {
        initCall(regPtr);
        Wire.write(newSetting);
        endCall();
    }
}

/*==============================================================================================================*
    SET PIN DATA
 *==============================================================================================================*/

void PCA9536::setPin(pin_t pin, reg_ptr_t regPtr, byte newSetting) {
    byte newReg = getReg(regPtr);
    bitWrite(newReg, pin, newSetting);
    setReg(regPtr, newReg);
}

/*==============================================================================================================*
    SET PIN DATA for 
 *==============================================================================================================*/

void PCA9536::setPins(reg_ptr_t regPtr, byte newSetting0, byte newSetting1, byte newSetting2, byte newSetting3) {
    byte newReg = getReg(regPtr);
    bitWrite(newReg, IO0, newSetting0);
    bitWrite(newReg, IO1, newSetting1);
    bitWrite(newReg, IO2, newSetting2);
    bitWrite(newReg, IO3, newSetting3);
    setReg(regPtr, newReg);
}

/*==============================================================================================================*
    INITIATE I2C COMMUNICATION
 *==============================================================================================================*/

void PCA9536::initCall(reg_ptr_t regPtr) {
    Wire.beginTransmission(DEV_ADDR);
    Wire.write(regPtr);
}

/*==============================================================================================================*
    END I2C COMMUNICATION
 *==============================================================================================================*/

void PCA9536::endCall() {
    _comBuffer = Wire.endTransmission();
}

/*==============================================================================================================*
    GET COMMUNICATION RESULT
 *==============================================================================================================*/

byte PCA9536::getComResult() {
    return _comBuffer;
}
