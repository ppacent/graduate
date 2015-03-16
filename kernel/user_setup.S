@ user_setup.S: helper function written in asm called from kernel
@
@ Author: Will Ross <wjross@andrew.cmu.edu>
@ Author: Peter Pacent <ppacent@andrew.cmu.edu>
@ Date:   11/12/14
@
@ Hint:
@   CPSR:
@       N|Z|C|V| [27:9] |I|F|T|M[4:0]
@   I -> 1
@   F -> 1
@   T -> T
@   CPSR_SVC = 0x10011 -> CPSR_USER = 0x10000

    .extern user_setup_stack_ptr

    .file   "user_setup.S"
    .text

    .global user_setup
user_setup:
    stmfd sp!, {r0-r12, lr}
    mrs r2, cpsr
    bic r2, #0xdf
    orr r2, r2, #0xd0
    bic r2, #0x80

    msr cpsr, r2
    ldmfd sp!, {r2}
    mov sp, r0
    mov pc, #0xa0000000
    nop
    nop
    nop
    nop





    @ Save gp registers, lr, and cpsr so that we can return to kernel main later.
    @stmfd   sp!, {r1-r12, lr}
    @mrs     r2, cpsr
    @stmfd   sp!, {r2}

    @ Save the stack pointer in a global variable for exit_handler.S.
    @ldr     r2, =user_setup_stack_ptr
    @str     sp, [r2] @ *user_setup_stack_ptr = sp

    @ change mode to USER
    @mrs     r2, cpsr
    @bic     r2, #0xFF @ Clear I, F, T, and Mode flags in CPSR
    @orr     r2, #0x50  @ Change mode bits to user and enable IRQs.
    @msr     cpsr, r2

    @ set up user stack from spTop passed in from kernel.c
    @mov     sp, r0

    @ go to user program
    @ldr     pc, =0xa0000000

    @@@ exit syscall essentially returns through here!
