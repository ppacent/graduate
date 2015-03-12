@ swi_handler.S: SWI Handler
@
@ Author: Will Ross <wjross@andrew.cmu.edu>
@ Author: Peter Pacent <ppacent@andrew.cmu.edu>
@ Date:   11/12/14
@
@ Hint: To change the SWI handler in class, we need to accommodate a return
@   value from the C_SWI_Handler.

    .extern C_SWI_Handler

    .file   "swi_handler.S"
    .text

    .global swi_handler
swi_handler:
    @ sub     sp, sp, #4
    stmfd   sp!, {r0-r12, lr}

    @ get spsr into general purpose (gp) registers
    @ mrs     r2, spsr

    @ store spsr above gp registers
    str     r2, [sp, #14*4]

    @ Set Param 1: extract SWI and get SWI no. by bit-masking
    ldr     r0, [lr, #-4]
    bic     r0, r0, #0xff000000

    @ Set Param 2: pointer to parameters on stack
    mov     r1, sp

    @ go into Supervisor mode with interrupts enabled  
    msr     cpsr_c, #0x13

    @ go to handler
    bl      C_SWI_Handler

    @ Disable interrupts
    msr     cpsr_c, #0xD3

    @ restore spsr
    @ ldr     r2, [sp, #14*4]
    @ msr     spsr, r2

    @ Discard the r0 on the stack (r0 has ret value).
    add     sp, sp, #4
    ldmfd   sp!, {r1-r12, lr}

    @ remove space used to store spsr
    @ add     sp, sp, #4
    movs    pc, lr
