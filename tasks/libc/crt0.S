/** @file crt0.S
 *
 * @brief Execution startup routine
 *
 * @author Peter Pacent <ppacent@andrew.cmu.edu>
 */

#include <asm.h>

	.file "crt0.S"
FUNC(_start)
	@ needs code.  Put your names at the top.
    @ For testing swis: 
    @SWI 0x900004
    @SWI 0x900003
    ldr r0, [sp]   @ give argc
    add r1, sp, #4 @ give argv
    bl main
loop:
	b loop
