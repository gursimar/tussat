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
// serial.c: implementation of the serial module.
//
//////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

#include "seos.h"
#include "alloc.h"
#include "serial.h"

//////// Internal Global Variable Declarations ////////

    //stores data received on the serial port
static volatile uint8_t *_rxqueue = NULL;
    //address of the byte at the end of the receive queue
static volatile uint8_t *_rxedge = NULL;
    //start pointer for the receive queue
static volatile uint8_t *_rxstart = NULL;
    //end pointer for the receive queue
static volatile uint8_t *_rxend = NULL;

    //mutex used for blocking in serialReadLineBlock()
static volatile seosMutex_t _serialmutex;

//////// Internal Function Declarations ////////

    //ISR for receiving serial data
void SIG_UART_RECV (void) __attribute__ ((signal));


//////////////////////////////////////////////////////////////////////
// serial ISR
//////////////////////////////////////////////////////////////////////

    //receive a byte from the serial port
void SIG_UART_RECV (void)
{
    uint8_t brecv;

    seosEnterIsr();
        
    brecv = UDR;    //receive the character from the serial port

        //if the receive buffer is not full
    if ((_rxstart != _rxqueue || _rxend != _rxedge) &&
        (_rxend != (_rxstart - 1))) {
            //add the byte to the receive queue
        *_rxend = brecv;
            //increment the end ptr of the receive queue
        _rxend = (_rxend == _rxedge) ? _rxqueue : (_rxend + 1);
            //release the serial mutex if a '\r' is received
        if (brecv == '\r' && _serialmutex.ntakes > 1)
            seosMutexRelease((seosMutex_t *)(&_serialmutex));
    } else {
            //if the receive buffer is full, release the serial mutex
        if (_serialmutex.ntakes > 1)
            seosMutexRelease((seosMutex_t *)(&_serialmutex));
    }

    seosExitIsr();
}


//////////////////////////////////////////////////////////////////////
// serial functions
//////////////////////////////////////////////////////////////////////

    //initializes the serial port (this should only be called once)
uint8_t serialInitialize(uint16_t baudrate, uint16_t rxqueuesize)
{

    cli();  //disable interrupts

        //set the baud rate
    UBRRH = (uint8_t)(baudrate >> 8);
    UBRRL = (uint8_t)baudrate;

        //initialize the receive queue
    if ((_rxqueue = (uint8_t *)allocNew(rxqueuesize+1)) == NULL)
        return(0);
    _rxedge = _rxqueue + rxqueuesize;   //last byte of the queue
        //initialize the start and end ptrs for the receive queue
    _rxstart = _rxqueue;
    _rxend = _rxstart;

        //initialize the USART control and status register A
        //(initialize the tx buffer to empty -- UDRE = 1)
    UCSRA = (1<<UDRE);

        //initialize the USART control and status register B
        //(enable the rx interrupt and turn on the rx/tx)
    UCSRB = (1<<RXCIE) | (1<<RXEN) | (1<<TXEN);

        //initialize the USART control and status register C
        //(use asyc mode, no parity, 1 stop bit, and 8 data bits)
    UCSRC = (1<<URSEL) | (3<<UCSZ0);

        //initialize the mutex used to block in serialReadLineBlock()
    seosMutexInitialize((seosMutex_t *)(&_serialmutex));
        //initially take to cause blocking
    seosMutexTake((seosMutex_t *)(&_serialmutex));

    sei();  //enable interrupts

    return(1);  //initialization successful
}

void serialWriteByte(uint8_t b)
{

        //wait for empty transmit buffer (UDRE == 1)
    while ((UCSRA & (1 << UDRE)) == 0);

        //set the serial data buffer (transmit the byte)
    UDR = b;
}

void serialWriteN(const uint8_t *bytes, uint16_t nbytes)
{
    uint16_t i;

    if (bytes == NULL || nbytes == 0)
        return;

        //write the data to the serial port
    for (i = 0; i < nbytes; i++)
        serialWriteByte(bytes[i]);
}

void serialWriteLine(const char *str)
{
    char *ptr;

    if (str == NULL)
        return;

        //write out the characters of the string
    for (ptr = (char *)str; *ptr != '\0'; ptr++) {
        if (*ptr == '\r' || *ptr == '\n')
            break;
        serialWriteByte(*ptr);
    }

        //add the \r\n
    serialWriteByte('\r');
    serialWriteByte('\n');
}

uint8_t serialReadByte(void)
{
    uint8_t nextbyte;

        //check if the receive queue is empty
    if (_rxstart == _rxend)
        return(0);

        //get the next byte in the receive queue
    nextbyte = *_rxstart;

        //increment the start ptr of the receive queue
    _rxstart = (_rxstart == _rxedge) ? _rxqueue : (_rxstart + 1);

    return(nextbyte);
}

uint16_t serialReadN(uint8_t *buffer, uint16_t nbytes)
{
    uint16_t nread = 0;

    if (buffer == NULL || nbytes == 0)
        return(0);

        //read nbytes from the receive queue
    for (nread = 0; nread < nbytes; nread++) {
        if ((buffer[nread] = serialReadByte()) == 0)
            break;
    }

    return(nread);
}

uint16_t serialReadLine(uint8_t *buffer, uint16_t buflen)
{
    uint8_t nextbyte;
    uint16_t nread;

    if (buffer == NULL || buflen == 0)
        return(0);

        //read a line from the receive queue
    for (nread = 0; nread < (buflen - 1); nread++) {
        nextbyte = serialReadByte();
        buffer[nread] = nextbyte;
        if (nextbyte == '\r' || nextbyte == 0)
            break;
    }

        //make sure the buffer is null terminated
    buffer[nread] = 0;

    return(nread);
}

uint16_t serialReadLineBlock(uint8_t *buffer, uint16_t buflen)
{
    uint8_t nextbyte;
    uint16_t nread;

    if (buffer == NULL || buflen == 0)
        return(0);

        //read a line from the receive queue
        //(block until a '\n' is read or the buffer is full)
    for (nread = 0; nread < (buflen - 1); nread++) {
        if ((nextbyte = serialReadByte()) == 0) {
                //block until '\n' is received
            seosMutexTake((seosMutex_t *)(&_serialmutex));
            nextbyte = serialReadByte();
        }
        if ((buffer[nread] = nextbyte) == '\r')
            break;
    }

        //make sure the buffer is null terminated
    buffer[nread] = 0;

    return(nread);
}

//////////////////////////////////////////////////////////////////////
// internal serial functions
//////////////////////////////////////////////////////////////////////

