/** @file time.S
 *
 * @brief time sycall wrapper
 *
 * @author Will Ross <wjross@andrew.cmu.edu>
 * @author Peter Pacent <ppacent@andrew.cmu.edu>
 * @date 2014-11-12
 */

#include <asm.h>
#include <bits/swi.h>

	.file "time.S"

FUNC(time)
	push {lr} @ store lr prior to swi call
    swi TIME_SWI @make the system call

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
