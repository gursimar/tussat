;-----------------------------------------------------------------------------
; Copyright (c) 2003, Oren Avissar
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;
; * Redistributions of source code must retain the above copyright
;   notice, this list of conditions and the following disclaimer.
; * Redistributions in binary form must reproduce the above copyright
;   notice, this list of conditions and the following disclaimer in
;   the documentation and/or other materials provided with the
;   distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.
;
; Assembly support functions for the seos (seos.c) module
;
; Built using avr-gcc version 3.3 20030421 (prerelease) packaged 
; with WinAVR 20030424.
;
;-----------------------------------------------------------------------------
.file   "seosasm.S"

;-----------------------------------------------------------------------------
; Start of seosasm Module
.section .text
;-----------------------------------------------------------------------------

; define constants
__SREG__ = 0x3f     ;status register
__SP_H__ = 0x3e     ;high part of the stack pointer
__SP_L__ = 0x3d     ;low part of the stack pointer
__tmp_reg__ = 0     ;temporary register


;-----------------------------------------------------------------------------
; Do the context switch
;
; NOTE: The first 40 bytes of the seosTask_t struct are organized as follows:
;       regs[32]    -> bytes 0-31 are used to store the regs
;       sreg        -> byte 32 is used to store the status register
;       sp          -> bytes 33-34 are used to store the stack pointer
;       state       -> byte 35 is the current state of the task
;       run()       -> bytes 36-37 store the entry point of the task
;       stack       -> bytes 38-39 store the stack buffer for the task
;
; This function assumes that the first parameter will be passed in registers
; r24 & r25 and the second parameter will be passed in r22 & r23 -- with the
; lower byte in the lower register.
;
; All registered are backed up and restored when doing a context switch.
; To optimize this the saving and restoring of registers that can be
; changed within a called functions can be removed.
;
; void seosasmContextSwitch(seosTask_t *oldcontext, seosTask_t *newcontext)
    .global	seosasmContextSwitch
    .type   seosasmContextSwitch, @function
seosasmContextSwitch:

    ;---------------------------------------------------------------
    ; Save the old context
    ; the parameter "oldcontext" is passed in using r24 & r25
    ;
    ; save the contents of the Z register (r30 & r31)
    push r30
    push r31
    ;
    ; move the "oldcontext" pointer into the Z register (r30 & r31)
    mov r30,r24
    mov r31,r25
    ;
    ; save the first 30 registers
	std Z+0,r0
	std Z+1,r1
	std Z+2,r2
	std Z+3,r3
	std Z+4,r4
	std Z+5,r5
	std Z+6,r6
	std Z+7,r7
	std Z+8,r8
	std Z+9,r9
	std Z+10,r10
	std Z+11,r11
	std Z+12,r12
	std Z+13,r13
	std Z+14,r14
	std Z+15,r15
	std Z+16,r16
	std Z+17,r17
	std Z+18,r18
	std Z+19,r19
	std Z+20,r20
	std Z+21,r21
	std Z+22,r22
	std Z+23,r23
	std Z+24,r24
	std Z+25,r25
	std Z+26,r26
	std Z+27,r27
	std Z+28,r28
	std Z+29,r29
	;
	; move Z to Y so we can resore Z
    mov r28,r30
    mov r29,r31
    pop r31
    pop r30
    ; save Z
    std Y+30,r30
    std Y+31,r31
    ;
    ; save the status register
    in __tmp_reg__,__SREG__
    std Y+32,__tmp_reg__
    ;
    ; save the stack pointer
    in __tmp_reg__,__SP_L__
    std Y+33,__tmp_reg__
    in __tmp_reg__,__SP_H__
    std Y+34,__tmp_reg__
    ;
    ; update the state of the old task to SEOS_TASK_READY (1)
    ldi r30,lo8(1)
    std Y+35,r30
    ;---------------------------------------------------------------


    ;---------------------------------------------------------------
    ; Move the "newcontext" pointer into the Z register (r30 & r31)
    ; the parameter "newcontext" is passed in using r22 & r23
    ;
    mov r30,r22
    mov r31,r23

    ;---------------------------------------------------------------
    ; Check if the task is being run for the first time (newly created)
    ;
    ; test if newcontext->state == SEOS_TASK_CREATED (0)
    ldd __tmp_reg__,Z+35
    tst __tmp_reg__
    brne .alreadycreated
    
    ;---------------------------------------------------------------
    ; The task is being run for the first time
    ;
    ; update the state of the task to SEOS_TASK_RUNNING (2)
    ldi r28,lo8(2)
    std Z+35,r28
    ;
    ; set the stack pointer for the new task
    ldd __tmp_reg__,Z+33
    out __SP_L__,__tmp_reg__
    ldd __tmp_reg__,Z+34
    out __SP_H__,__tmp_reg__
    ;
    ; On the AVR interrupts are dissabled when an interrupt occurs
    sei     ;re-enable interrupts if necessary
    ;
    ; save the pointer to the new context on the stack
    push r22
    push r23
    ;
    ; start the new task (pass the parameter in r24 & r25)
    mov r24,r22
    mov r25,r23
    ; call newcontext->run()
    ldd r28,Z+36
    ldd r29,Z+37
    mov r30,r28     ;move the newcontext->run() pointer into Z
    mov r31,r29
    icall           ;call newcontext->run()
    ;
    ; If the task returns, restore the new context pointer
    ; -- pop the values into the Y register
    pop r29
    pop r28
    ;
    ; switch back to the default context
    lds r30,seos_deftask    ;move seos_deftask into Z
    lds r31,(seos_deftask)+1
    ;    
    ; restore the stack pointer
    ldd __tmp_reg__,Z+33
    out __SP_L__,__tmp_reg__
    ldd __tmp_reg__,Z+34
    out __SP_H__,__tmp_reg__
    ;
    ; remove returned task from the scheduler's data structures.
    ; call _seosTaskRemove(&seos_tasktop,newcontext)
    mov r22,r28             ;move newcontext into r22 & r23
    mov r23,r29
    ldi r24,lo8(seos_tasktop)   ;move &seos_tasktop into r24 & r25
    ldi r25,hi8(seos_tasktop)
    push r28    ;save the Y register (may not be necessary w/avr-gcc)
    push r29
    call _seosTaskRemove
    pop r29     ;restore the Y register
    pop r28
    ;
    ; free the stack for the task
    ; call allocDelete(newcontext->stack)
    ldd r24,Y+38
    ldd r25,Y+39
    push r28    ;save the Y register (may not be necessary w/avr-gcc)
    push r29
    call allocDelete
    pop r29     ;restore the Y register
    pop r28
    ;
    ; free the task information
    ; call allocDelete(newcontext)
    mov r24,r28
    mov r25,r29
    call allocDelete
    ;
    ; reload the default context (seos_deftask) into Z
    lds r30,seos_deftask
    lds r31,(seos_deftask)+1
    ;
    ; set the currently running task to the default context
    ; -- set seos_runningtask = seos_deftask
    sts seos_runningtask,r30
    sts (seos_runningtask)+1,r31
    ;---------------------------------------------------------------

.alreadycreated:


    ;---------------------------------------------------------------
    ; Set the new context (stored in the Z register)
    ;
    ; update the state of the task to SEOS_TASK_RUNNING (2)
    ldi r28,lo8(2)
    std Z+35,r28
    ;
    ; set the new stack pointer
    ldd __tmp_reg__,Z+33
    out __SP_L__,__tmp_reg__
    ldd __tmp_reg__,Z+34
    out __SP_H__,__tmp_reg__
    ;
    ; On the AVR interrupts are dissabled when an interrupt occurs
    sei     ;re-enable interrupts if necessary
    ;    
    ; set the new status register
    ldd __tmp_reg__,Z+32
    out __SREG__,__tmp_reg__
    ;
    ; set the first 30 registers
	ldd r0,Z+0
	ldd r1,Z+1
	ldd r2,Z+2
	ldd r3,Z+3
	ldd r4,Z+4
	ldd r5,Z+5
	ldd r6,Z+6
	ldd r7,Z+7
	ldd r8,Z+8
	ldd r9,Z+9
	ldd r10,Z+10
	ldd r11,Z+11
	ldd r12,Z+12
	ldd r13,Z+13
	ldd r14,Z+14
	ldd r15,Z+15
	ldd r16,Z+16
	ldd r17,Z+17
	ldd r18,Z+18
	ldd r19,Z+19
	ldd r20,Z+20
	ldd r21,Z+21
	ldd r22,Z+22
	ldd r23,Z+23
	ldd r24,Z+24
	ldd r25,Z+25
	ldd r26,Z+26
	ldd r27,Z+27
	ldd r28,Z+28
	ldd r29,Z+29
    ;
    ; save Y
    push r28
    push r29
    ;
	; move Z to Y so we can set Z
    mov r28,r30
    mov r29,r31
	ldd r30,Y+30
	ldd r31,Y+31
	;
	; restore Y
	pop r29
	pop r28
    ;---------------------------------------------------------------

    ret
    .size   seosasmContextSwitch, .-seosasmContextSwitch
; END OF seosasmContextSwitch()
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; This is called when there are no other tasks to run
; -- when the OS is idle.
;
; void seosasmIdle(void)
    .global	seosasmIdle
    .type   seosasmIdle, @function
seosasmIdle:

; 	while (1);
.donothing:
    rjmp .donothing

    ret
    .size   seosasmIdle, .-seosasmIdle
; END OF seosasmIdle()
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; End of seosasm Module
.end
;-----------------------------------------------------------------------------
