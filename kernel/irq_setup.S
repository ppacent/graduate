@ irq_setup.S: helper function written in asm called from kernel
@
@ Author: Will Ross <wjross@andrew.cmu.edu>
@ Author: Peter Pacent <ppacent@andrew.cmu.edu>
@ Date:   Wed, 29 Oct 2014
@


    .file   "irq_setup.S"
    .text

    .global irq_setup
irq_setup:
    @ Switch to IRQ mode
    mrs r1, cpsr
    bic r2, r2, #0xff
    orr r2, r2, #0x92
    msr cpsr, r2

    @ Setup stack 
    ldr sp, =0xa2000000

    @ return to supervisor mode
    msr cpsr, r1
    mov pc, lr
