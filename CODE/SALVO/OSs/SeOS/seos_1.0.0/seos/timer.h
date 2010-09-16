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
// timer.h: interface for the timer module.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>

    //set the value for Timer 0 to create a 1 ms (0.001 sec) delay
    //256 - (clk * .001)/64 = TCNT0 -> 3686400 Hz clk using clk/64 (prescaler)
#define TIMER_TCNT0_VALUE 198   //round 57.6 to 58 -> 256 - 58 = 198

#define TIMER_TPYE_ONESHOT  1
#define TIMER_TYPE_PERIODIC 2

#define TIMER_STATE_IDLE   1
#define TIMER_STATE_ACTIVE 2
#define TIMER_STATE_DONE   3

typedef struct _timer_t {
    volatile uint8_t state;     //state of the timer (idle, active, done)
    volatile uint8_t type;      //type of the timer (one shot, periodic)
    volatile uint16_t length;   //length of the timer (in ms -> 0.001 sec)
    volatile uint16_t count;    //number of ticks (cs) remaining
    volatile seosMutex_t mutex; //used to block on the timer
    struct _timer_t *next;      //pointer to the next timer
} timer_t;

//////// Timer Functions ////////

timer_t *timerStart(uint16_t timems, uint8_t type);
void timerWaitFor(timer_t *handle);
void timerCancel(timer_t *handle);

#endif //TIMER_H
