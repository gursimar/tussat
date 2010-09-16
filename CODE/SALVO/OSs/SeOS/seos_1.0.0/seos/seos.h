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
// seos.h: interface for the seos module
//         (Simple Embedded Operating System).
//
//////////////////////////////////////////////////////////////////////////////
#ifndef SEOS_H
#define SEOS_H

#include <inttypes.h>

#define SEOS_STATE_UNINITIALIZED  0
#define SEOS_STATE_INITIALIZED    1
#define SEOS_STATE_STARTED        2

#define SEOS_TASK_CREATED 0
#define SEOS_TASK_READY   1
#define SEOS_TASK_RUNNING 2
#define SEOS_TASK_WAITING 3

typedef struct _seosContext_t {
    uint8_t regs[32];   //stores the general purpose regs
    uint8_t sreg;       //stores the status register
    uint8_t *sp;        //current stack pointer for the task
} seosContext_t;

typedef struct _seosTask_t {
    seosContext_t context;  //context for the task
    uint8_t state;          //state of the task
    void (*run)(void);      //entry point for the task - function pointer
    uint8_t *stack;         //stack space for the task
    uint8_t priority;       //priority for the task
    struct _seosTask_t *next; //pointer to the next task
} seosTask_t;

typedef struct _seosMutex_t {
    uint8_t ntakes;         //number of outstanding takes for the mutex
    seosTask_t *waitlist;   //list of tasks waiting on the mutex
} seosMutex_t;

union seosInt_u {
    uint16_t intval;
    uint8_t bytes[sizeof(int)];
};

//////// SEOS Functions ////////

void seosInitialize(void);
void seosStart(void);

seosTask_t *seosTaskAdd(void (*run)(void), uint8_t priority, uint16_t stacksize);

void seosMutexInitialize(seosMutex_t *mutex);
void seosMutexTake(seosMutex_t *mutex);
void seosMutexRelease(seosMutex_t *mutex);
void seosMutexDestroy(seosMutex_t *mutex);

    //must be called at the start of any interrupt that could cause a CS
void seosEnterIsr(void);
    //must be called at the end of any interrupt that could cause a CS
void seosExitIsr(void);

//////// Global internal SEOS functions (called in seosasm.S) ////////

    //add a new task
void _seosTaskInsert(seosTask_t **ptasktop, seosTask_t *newtaskinfo);
    //remove a task
void _seosTaskRemove(seosTask_t **ptasktop, seosTask_t *taskinfo);

#endif //SEOS_H
