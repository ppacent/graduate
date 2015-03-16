/** @file write.S
 *
 * @brief write sycall wrapper
 *
 * 
 * @author Peter Pacent <ppacent@andrew.cmu.edu>
 */

#include <asm.h>
#include <bits/swi.h>

    .file "read.S"

FUNC(read)
    push {lr} @ store lr prior to swi call
    swi READ_SWI @make the system call

    @ Return on success

    cmp r0, #0 @success
    blt error
    pop {lr}
    mov pc, lr @pl means >= 0
error:
    @ Store the absolute value of return value in errno
    neg r0, r0
    ldr r1, =errno
    str r0, [r1]

    @ Store -1 in return value
    mov r0, #-1
    mov pc, lr
