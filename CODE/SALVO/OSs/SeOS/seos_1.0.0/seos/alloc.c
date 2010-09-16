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
// alloc.c: implementation of the alloc module.
//
// Description: This module dynamically allocated blocks of memory
//              in SRAM.  The format of each allocation block is:
//
//                2 bytes  2 bytes      "nbytes"
//               -----------------------------------
//              |  next  |  nbytes | block data ... |
//               -----------------------------------
//
//              "next" is a pointer to the next alloc block.  If
//              it is NULL, there is no next block.  "nbytes" is
//              the size of the allocation.  "block data" is the
//              actual block of memory that is returned (its
//              size is "nbytes").
//
//////////////////////////////////////////////////////////////////////////////

#include "alloc.h"

    //an invalid data address (use NULL)
#define _ADDR_INVALID NULL

//////// Internal Global Variable Declarations ////////

    //stores the total number of bytes free
static uint16_t _bytesFree = ALLOC_MEMSIZE - sizeof(int);

    //static buffer to use for memory allocation
static uint8_t _sramBuffer[ALLOC_MEMSIZE - sizeof(int)];


//////////////////////////////////////////////////////////////////////
// xalloc functions
//////////////////////////////////////////////////////////////////////

    //reset the memory allocation buffer
void allocReset(void)
{

        //reset the allocation buffer
    _bytesFree = sizeof(_sramBuffer);
}

    //allocate a new block of memory
    //on error NULL is returned.
    //NOTE: it is ok to use NULL as an invalid return value because
    //      the returned pointer will always be >= sizeof(alloc_t)
void *allocNew(uint16_t nbytes)
{
    alloc_t *blockinfo;
    alloc_t *insertptr;

        //check if there is enough space for the allocation
        //NOTE: there is sizeof(alloc_t) bytes of overhead per allocation
    if (nbytes > (_bytesFree - sizeof(alloc_t)) || _bytesFree < sizeof(alloc_t))
        return(NULL);

        //if blocks have previously been allocated
    if (_bytesFree < sizeof(_sramBuffer)) {
        insertptr = (alloc_t *)(&_sramBuffer);
        blockinfo = NULL;
            //look for free gaps in memory
        while (insertptr->next != NULL) {
                //check if there is a gap big enough for the allocation
            if (((uint16_t)insertptr + sizeof(alloc_t) + insertptr->nbytes +
                sizeof(alloc_t) + nbytes) <= (uint16_t)(insertptr->next)) {
                    //allocate the new block
                blockinfo = (alloc_t *)((uint16_t)insertptr + sizeof(alloc_t) + insertptr->nbytes);
                    //update the "next" pointers
                blockinfo->next = insertptr->next;
                insertptr->next = blockinfo;
                break;  //a big enough gap was found
            }
            insertptr = insertptr->next;
        }
            //if no gaps were found, try to allocate at the end
        if (blockinfo == NULL) {
                //check if there is enough room at the end
            if (((uint16_t)insertptr + sizeof(alloc_t) + insertptr->nbytes +
                sizeof(alloc_t) + nbytes) <= ((uint16_t)_sramBuffer + sizeof(_sramBuffer))) {
                blockinfo = (alloc_t *)((uint16_t)insertptr + sizeof(alloc_t) + insertptr->nbytes);
                    //update the "next" pointers
                blockinfo->next = NULL; //this is last allocated block
                insertptr->next = blockinfo;
            } else {
                    //there is no contiguous block of memory that is big enough
                return(NULL);
            }
        }
    } else {
            //allocate the block at the start of memory
        blockinfo = (alloc_t *)(&_sramBuffer);
            //set the pointer to the next block
        blockinfo->next = NULL; //this is last allocated block
    }

        //set the number of bytes being allocated
    blockinfo->nbytes = nbytes;
        //update the number of free bytes
    _bytesFree -= (nbytes + sizeof(alloc_t));

        //return a pointer to the allocated block of memory
    return((void *)((uint16_t)blockinfo+sizeof(alloc_t)));
}

    //free an allocated block of memory
void allocDelete(void *memptr)
{
    alloc_t *blockinfo;
    alloc_t *previnfo = (alloc_t *)_ADDR_INVALID;

    if (memptr == NULL)
        return; //if allocNew() failed

        //get a pointer to the previous allocation block
    blockinfo = (alloc_t *)(&_sramBuffer);   //start at the beginning
    while ((uint16_t)blockinfo != ((uint16_t)memptr - sizeof(alloc_t)) && blockinfo->next != NULL) {
        previnfo = blockinfo;
        blockinfo = blockinfo->next;
    }

    if ((uint16_t)blockinfo != ((uint16_t)memptr - sizeof(alloc_t)))
        return; //the specified memory block was not found

        //no other blocks are allocated
    if (previnfo == (alloc_t *)_ADDR_INVALID) {
            //reset the allocation buffer
        _bytesFree = sizeof(_sramBuffer);
    } else {
            //set the next ptr of the previous block to
            //the next ptr of the current block
        previnfo->next = blockinfo->next;
            //update the total number of free bytes
        _bytesFree += (blockinfo->nbytes + sizeof(alloc_t));
    }
}

//////////////////////////////////////////////////////////////////////
// internal xalloc functions
//////////////////////////////////////////////////////////////////////
