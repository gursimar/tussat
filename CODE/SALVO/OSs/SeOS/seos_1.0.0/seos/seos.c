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
// seos.c: implementation of the seos module.
//
//////////////////////////////////////////////////////////////////////////////

#include <avr/interrupt.h>

#include "seos.h"
#include "seosasm.h"
#include "alloc.h"


//////// Global Variable Declarations (used in seosasm.S) ////////

    //top of the task list
seosTask_t *seos_tasktop = NULL;
    //information for the currently running task
seosTask_t *seos_runningtask = NULL;

    //stores the context for the default task
    //(main thread running at startup)
seosTask_t *seos_deftask = NULL;

//////// Internal Global Variable Declarations ////////

    //the current state of the OS
static uint8_t _state = SEOS_STATE_UNINITIALIZED;

    //nesting level of interrupts
static uint8_t _interruptlevel = 0;
    //flag to check if sched should be done after interrupt
static uint8_t _flagsched = 0;

//////// Internal Function Declarations ////////

    //schedule the tasks
static void Schedule(void);


//////////////////////////////////////////////////////////////////////
// seos functions
//////////////////////////////////////////////////////////////////////

    //this should only be called once
void seosInitialize(void)
{

    if (_state != SEOS_STATE_UNINITIALIZED)
        return;

        //enable interrupts
    sei();

        //initialize dynamic memory allocation
    allocReset();

        //create the default task information struct
    if ((seos_deftask = (seosTask_t *)allocNew(sizeof(seosTask_t))) == NULL)
        return;
        //initialize the task-specific data.
    seos_deftask->context.sp = NULL;
    seos_deftask->state = SEOS_TASK_RUNNING;
    seos_deftask->run = NULL;
    seos_deftask->stack = NULL;
    seos_deftask->priority = 0;
    seos_deftask->next = NULL;

        //set the default task to the current running task
    seos_runningtask = seos_deftask;

        //set the os state to initialized
    _state = SEOS_STATE_INITIALIZED;

        //add the seosasmIdle() task w/ lowest (0) priority
        //NOTE: seosasmIdle() has a stack size of 32 bytes 
        //      (needed for function call overhead --including ISRs)
    seosTaskAdd(seosasmIdle,0,32);
}

    //this should only be called once
void seosStart(void)
{

    if (_state != SEOS_STATE_INITIALIZED)
        return;

    _state = SEOS_STATE_STARTED;

    Schedule();     //scheduling point

    // This line will never be executed.
}

seosTask_t *seosTaskAdd(void (*run)(void), uint8_t priority, uint16_t stacksize)
{
    seosTask_t *taskinfo;

    if (_state == SEOS_STATE_UNINITIALIZED)
        return(NULL);

        //create the struct to store information about the task
    if ((taskinfo = (seosTask_t *)allocNew(sizeof(seosTask_t))) == NULL)
        return(NULL);

        //allocate the stack to be used by the task
    if ((taskinfo->stack = (uint8_t *)allocNew(stacksize)) == NULL) {
        allocDelete(taskinfo);
        return(NULL);
    }

        //initialize the task-specific data.
        //NOTE: the stack pointer on the AVR is post-decremented
    taskinfo->context.sp = taskinfo->stack + stacksize - 1;
    taskinfo->run = run;
    taskinfo->state = SEOS_TASK_CREATED;  //set the task to newly created
    taskinfo->priority = priority;
    taskinfo->next = NULL;

        //insert the task into the ready list.
    _seosTaskInsert(&seos_tasktop,taskinfo);

    Schedule();     //scheduling point

    return(taskinfo);
}

void seosMutexInitialize(seosMutex_t *mutex)
{

        //initialize the mutex to available
    mutex->ntakes = 0;
        //initialize the waitlist for the mutex
    mutex->waitlist = NULL;
}

void seosMutexTake(seosMutex_t *mutex)
{

    if (mutex->ntakes == 0) {
            //the mutex is available.  simply take it and return.
        mutex->ntakes = 1;
        mutex->waitlist = NULL;
    } else {
            //the mutex is taken.  add the calling task to the waiting list.
        seos_runningtask->state = SEOS_TASK_WAITING;        //set the current running task to waiting
        _seosTaskRemove(&seos_tasktop,seos_runningtask);    //remove the task from the readylist
        _seosTaskInsert(&(mutex->waitlist),seos_runningtask); //add the task to the waitlist
            //increment the number of outstanding takes for the mutex
        mutex->ntakes = (mutex->ntakes < 255) ? mutex->ntakes + 1 : 255;

        Schedule();     //scheduling point

        // When the mutex is released, the caller begins executing here.
    }
}

void seosMutexRelease(seosMutex_t *mutex)
{
    seosTask_t *taskinfo;

    if (mutex->ntakes > 0) {
        if (mutex->waitlist != NULL) {
                //set the information for the top waiting task
            taskinfo = mutex->waitlist;
                //move to the next waiting task
            mutex->waitlist = mutex->waitlist->next;
                //wake the top task on the waiting list.
            taskinfo->state = SEOS_TASK_READY;        //set the waiting task to ready
            _seosTaskInsert(&seos_tasktop,taskinfo);  //add the task to the readylist
                //decrement the number of outstanding takes for the mutex
            (mutex->ntakes)--;

            Schedule();         //scheduling point
        } else {
            mutex->ntakes = 0;  //mutex is available
        }
    }
}

void seosMutexDestroy(seosMutex_t *mutex)
{

        //put all waiting tasks back on the ready list
    while (mutex->waitlist != NULL) {
        mutex->waitlist->state = SEOS_TASK_READY;       //set the waiting task to ready
        _seosTaskInsert(&seos_tasktop,mutex->waitlist); //add the task to the readylist
        mutex->waitlist = mutex->waitlist->next;        //move to the next waiting task
    }
}

void seosEnterIsr(void)
{

    _interruptlevel++;  //increment the interrupt level
}

void seosExitIsr(void)
{

    _interruptlevel--;  //decrement the interrupt level

    if (_interruptlevel == 0 && _flagsched != 0) {
        _flagsched = 0;
        Schedule();     //scheduling point
    }
}


//////////////////////////////////////////////////////////////////////
// global internal seos functions (called from seosasm.S)
//////////////////////////////////////////////////////////////////////

void _seosTaskInsert(seosTask_t **ptasktop, seosTask_t *newtaskinfo)
{
    seosTask_t **ptaskinfo;

        //walk down the ordered list until a lower priority task is found.
    ptaskinfo = ptasktop;
    while (*ptaskinfo != NULL &&
           newtaskinfo->priority <= ((*ptaskinfo)->priority))
    {
        ptaskinfo = &((*ptaskinfo)->next);
    }

        //insert the new task into the list here.
    newtaskinfo->next = *ptaskinfo;
    *ptaskinfo = newtaskinfo;
}

void _seosTaskRemove(seosTask_t **ptasktop, seosTask_t *taskinfo)
{
    seosTask_t **ptaskinfo;

        //walk down the linked list until the dead task is found.
    ptaskinfo = ptasktop;
    while (*ptaskinfo != NULL) {
        if (*ptaskinfo == taskinfo)
            break;
        ptaskinfo = &((*ptaskinfo)->next);
    }

        //remove the task from the linked list
    if (*ptaskinfo != NULL)
        *ptaskinfo = taskinfo->next;
}


//////////////////////////////////////////////////////////////////////
// internal seos functions
//////////////////////////////////////////////////////////////////////

static void Schedule(void)
{
    seosTask_t *oldtaskinfo;

    if (_state != SEOS_STATE_STARTED)
        return;

        //postpone rescheduling until interrupts are completed
    if (_interruptlevel != 0) {
        _flagsched = 1;
        return;
    }

        //if there is a higher-priority ready task, switch to it.
    while (seos_runningtask != seos_tasktop) {
            //set the previous running task
        oldtaskinfo = seos_runningtask;
            //set the new running task
        seos_runningtask = seos_tasktop;
            //switch to the new task
        seosasmContextSwitch(oldtaskinfo,seos_tasktop);
    }
}
