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
// timer.c: implementation of the timer module.
//
// NOTE: Timer 0 is used to keep time
//
//////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

#include "seos.h"
#include "alloc.h"
#include "timer.h"

//////// Internal Global Variable Declarations ////////

    //has the timer initialization been done (0 = no, !0 = yes)
static volatile uint8_t _initDone = 0;

    //top of the list of timers
static volatile timer_t *_timerTop = NULL;

//////// Internal Function Declarations ////////

    //ISR for Timer/Counter0 Overflow
void SIG_OVERFLOW0 (void) __attribute__ ((signal));

    //initializes the hardware for the timer
static void Initialize(void);
    //add a new timer
static timer_t *Insert(timer_t *newtimernode);
    //remove a timer
static void Remove(timer_t *timernode);


//////////////////////////////////////////////////////////////////////
// Timer 0 ISR
//////////////////////////////////////////////////////////////////////

    //set the interrupt for Timer/Counter0 Overflow
void SIG_OVERFLOW0 (void)
{
    timer_t **ptoptimer;

    seosEnterIsr();

        //reload the timer value
    TCNT0 = TIMER_TCNT0_VALUE;

        //update the linked list of timers for a clock tick
    ptoptimer = (timer_t **)(&_timerTop);
    if (*ptoptimer != NULL) {
            //decrement the count on the head of the timer list
        (*ptoptimer)->count--;
            //mark all of the expired timers done and remove them
        while (*ptoptimer != NULL) {
            if ((*ptoptimer)->count > 0)
                break;
            (*ptoptimer)->state = TIMER_STATE_DONE;
            seosMutexRelease((seosMutex_t *)&((*ptoptimer)->mutex));
            *ptoptimer = (*ptoptimer)->next;
        }                                   
    }

    seosExitIsr();
}


//////////////////////////////////////////////////////////////////////
// timer functions
//////////////////////////////////////////////////////////////////////

timer_t *timerStart(uint16_t timems, uint8_t type)
{
    timer_t *newtimernode;

    if (_initDone == 0)
        Initialize();   //initialize the timer if necessary

        //allocate a new timer node
    if ((newtimernode = (timer_t *)allocNew(sizeof(timer_t))) == NULL)
        return(NULL);

    newtimernode->type = type;
    newtimernode->length = timems;
    newtimernode->state = TIMER_STATE_ACTIVE;

        //initialize the mutex
    seosMutexInitialize((seosMutex_t *)&(newtimernode->mutex));
        //take the mutex.  it will be released when the timer expires
    seosMutexTake((seosMutex_t *)&(newtimernode->mutex));

    return(Insert(newtimernode));
}

void timerWaitFor(timer_t *handle)
{

        //wait for the timer to expire
    seosMutexTake((seosMutex_t *)&(handle->mutex));

        //restart or idle the timer, depending on its type
    if (handle->type == TIMER_TYPE_PERIODIC) {
        handle->state = TIMER_STATE_ACTIVE;
        Insert(handle);
    } else {
        seosMutexRelease((seosMutex_t *)&(handle->mutex));
        handle->state = TIMER_STATE_IDLE;
    }
}

void timerCancel(timer_t *handle)
{

        //remove the timer from the timer list.
    if (handle->state == TIMER_STATE_ACTIVE)
        Remove(handle);

        //free the memory for the timer node
    allocDelete(handle);
}

//////////////////////////////////////////////////////////////////////
// internal timer functions
//////////////////////////////////////////////////////////////////////

static void Initialize(void)
{

        //initialize the timer 0 control register
    TCCR0 = (1 << CS01) | (1 << CS00);  //set prescaler to clk/64

        //initialize the timer counter
    TCNT0 = TIMER_TCNT0_VALUE;
    
        //set the Timer/Counter0 Overflow Interrupt Enable
	TIMSK |= (1 << TOIE0);

    _initDone = 1;  //timer initialization complete
}

static timer_t *Insert(timer_t *newtimernode)
{
    timer_t **ptimernode;
    
    TIMSK &= 0xFE;  //disable the interrupt for Timer 0

        //initialize the new timer's tick count
    newtimernode->count = newtimernode->length;

        //walk down the timer list, subtracting ticks as we go.
    ptimernode = (timer_t **)(&_timerTop);
    while (*ptimernode != NULL &&
           newtimernode->count >= (*ptimernode)->count)
    {
        newtimernode->count -= (*ptimernode)->count;
        ptimernode = &((*ptimernode)->next);
    }

        //insert the new timer at this point in the timer list.
    newtimernode->next = *ptimernode;
    *ptimernode = newtimernode;

        //adjust the tick count of the next timer (if any).
    if (newtimernode->next != NULL)
        newtimernode->next->count -= newtimernode->count;

    TIMSK |= 0x01;    //enable the interrupt for Timer 0

    return(newtimernode);
}

static void Remove(timer_t *timernode)
{
    timer_t **ptimernode;

    TIMSK &= 0xFE;  //disable the interrupt for Timer 0

        //walk down the linked list until the dead timer is found.
    ptimernode = (timer_t **)(&_timerTop);
    while (*ptimernode != NULL) {
        if (*ptimernode == timernode) 
            break;
        ptimernode = &((*ptimernode)->next);
    }

        //remove the dead timer from the linked list
    if (*ptimernode != NULL) {
        *ptimernode = timernode->next;
        if (timernode->next != NULL)
            timernode->next->count += timernode->count;
    }

    TIMSK |= 0x01;    //enable the interrupt for Timer 0
}
