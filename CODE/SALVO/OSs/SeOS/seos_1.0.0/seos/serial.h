//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003, Oren Avissar
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in
//   the documentation and/or other materials provided with the
//   distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// serial.h: interface for the serial module.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef SERIAL_H
#define SERIAL_H

#include <inttypes.h>

    //The values to load into TH1 for the corresponding baud rates.
    // UBRR = (fosc/(16*Baud)) - 1 -> where Crystal = 3686400
#define SERIAL_BAUDRATE_2400  95
#define SERIAL_BAUDRATE_4800  47
#define SERIAL_BAUDRATE_9600  23
#define SERIAL_BAUDRATE_14400 15
#define SERIAL_BAUDRATE_19200 11
#define SERIAL_BAUDRATE_28800 7
#define SERIAL_BAUDRATE_38400 5
#define SERIAL_BAUDRATE_57600 3

//////// Serial Functions ////////

uint8_t serialInitialize(uint16_t baudrate, uint16_t rxqueuesize);

void serialWriteByte(uint8_t b);
void serialWriteN(const uint8_t *bytes, uint16_t nbytes);
void serialWriteLine(const char *str);

uint8_t serialReadByte(void);
uint16_t serialReadN(uint8_t *buffer, uint16_t nbytes);
uint16_t serialReadLine(uint8_t *buffer, uint16_t buflen);
uint16_t serialReadLineBlock(uint8_t *buffer, uint16_t buflen);

#endif //SERIAL_H
