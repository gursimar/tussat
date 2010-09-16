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
// alloc.h: interface for the alloc module.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef ALLOC_H
#define ALLOC_H

#include <inttypes.h>

    //define NULL if necessary
#ifndef NULL
#define NULL 0
#endif

    //sets the amount of SRAM to use for dynamic memory allocation
#define ALLOC_MEMSIZE 800

typedef struct _alloc_t {
    struct _alloc_t *next;  //pointer to the next block information (alloc_t) struct
    uint16_t nbytes;        //number of bytes allocated in this block
} alloc_t;


void allocReset(void);
void *allocNew(uint16_t nbytes);
void allocDelete(void *memptr);

#endif //ALLOC_H
