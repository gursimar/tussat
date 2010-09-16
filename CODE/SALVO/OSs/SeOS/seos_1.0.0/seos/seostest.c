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
// SEOS test
//
//////////////////////////////////////////////////////////////////////////////

#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "seos.h"
#include "alloc.h"
#include "timer.h"
#include "serial.h"


void BlinkingLights(void)
{
    timer_t *timerhandle;

    timerhandle = timerStart(1000,TIMER_TYPE_PERIODIC);

    DDRB = 0xFF;    //initialize PORTB for output to LEDS
    PORTB = 0xFF;

    while (1) {
        PORTB = 0x00;
        timerWaitFor(timerhandle);
        PORTB = 0xFF;
        timerWaitFor(timerhandle);
    }
}

void Echo(void)
{
    uint8_t *buffer;

    serialWriteLine("Starting...");

    buffer = (uint8_t *)allocNew(100);

    while (1) {
        serialWriteN("SEOS> ",6);
        serialReadLineBlock(buffer,100);
        serialWriteN("\r\nSEOS> ",8);
        serialWriteLine(buffer);
    }
}


int main(void)
{

        //initialize the processor
	seosInitialize();

        //initialize the serial port
    serialInitialize(SERIAL_BAUDRATE_19200,80);

        //add the Echo() task
    seosTaskAdd(Echo,150,64);
        //add the BlinkingLights() task
    seosTaskAdd(BlinkingLights,100,64);

        //start the SEOS
    seosStart();

    // This line will never be executed.

    return(0);
}
