/** @file crt0.S
 *
 * @brief Execution startup routine
 *
 * @author Will Ross <wjross@andrew.cmu.edu>
 * @author Peter Pacent <ppacent@andrew.cmu.edu>
 * @date   Sun, 07 Oct 2007 01:51:29 -0400
 */

#include <asm.h>

	.file "crt0.S"

FUNC(_start)
	@ needs code.  Put your names at the top.
    ldr r0, [sp]   @ give argc
    add r1, sp, #4 @ give argv
    bl main
loop:
	b loop
