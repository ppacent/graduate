/** @file main.c
 *
 * @brief kernel main
 *
 * @author 
 * Will Ross <wjross@andrew.cmu.edu>
 * Peter Pacent <ppacent@andrew.cmu.edu>
 *	   
 * @date
 */
 
#include <kernel.h>
#include <task.h>
#include <sched.h>
#include <device.h>
#include <assert.h>

#include "arm/timer.h"
#include "arm/interrupt.h"
#include "arm/reg.h"
#include "arm/int_asm.h"

#include "bits/swi.h"
#include "bits/errno.h" 
#include "bits/fileno.h"

#include "sched.h"
#include "swi_handler.h"
#include "syscall.h"
#include "lock.h"
#include "irq_setup.h"

#include "user_setup.h"

#define SWI_VECT_ADDR 0x08
#define PC_OFFSET 0x08
#define IRQ_VECT_ADDR 0x18

 // Cannot write to this address. kernel.bin loaded here. Stack grows down.
#define USER_STACK_TOP 0xa3000000

// (LDR pc, [pc, 0x000]) 0xe59ff000 --> 0x000 through masking
#define LDR_PC_PC_INSTR 0xe59ff000
#define LDR_PC_PC4_INSTR 0xe51ff004
#define LDR_SIGN_MASK 0x00800000

#define BAD_CODE 0x0badc0de

#define SFROM_START 0x00000000
#define SFROM_END 0x00ffffff
#define SDRAM_START 0xa0000000
#define SDRAM_END 0xa3ffffff

uint32_t global_data;
volatile unsigned int os_time = 0;
int user_setup_stack_ptr;

/* Sleep for ms amount of time */
void sleep(unsigned int ms){
    printf("I'm in sleep \n");
	
    unsigned int current_time = os_time;
	while(1){ // loop until correct amount of time has passed 
		if (current_time + ms <= os_time){
			return;
		}
	}
}

/* Verifies that the buffer is entirely in valid memory. */
int check_mem(char *buf, int count, unsigned start, unsigned end) {
    unsigned start_buf = (unsigned) buf;
    unsigned end_buf = (unsigned)(buf + count);

    // Check if start of buff in bounds
    if ( (start_buf < start) || (start_buf > end) ) {
        return FALSE;
    }
    // Check if end of buf is in bounds
    if ( (end_buf < start) || (end_buf > end) ) {
        return FALSE;
    }
    // Overflow case.
    if ( start_buf >= end_buf ) {
        return FALSE;
    }

    return TRUE;
}

// write function to replace the system's write function
ssize_t write_handler(int fd, const void *buf, size_t count) {
    printf("I'm write_handler \n");    

    // Check for invalid memory range or file descriptors
    if (check_mem((char *) buf, (int) count, SDRAM_START, SDRAM_END) == FALSE &&
        check_mem((char *) buf, (int) count, SFROM_START, SFROM_END) == FALSE) {
        return EFAULT;
    // If file descriptor isn't stdout then we have an error
    } else if (fd != STDOUT_FILENO) {
        return EBADF;
    }

    char *buffer = (char *) buf;
    size_t i;
    char read_char;
    // Write into buffer one character at a time
    for (i = 0; i < count; i++) {
        // put character into buffer and putc
        read_char = buffer[i];
        putc(read_char);
    }
    return i;
}

// read function to replace the system's read function
ssize_t read_handler(int fd, void *buf, size_t count) {
    
    printf("I'm read_handler \n");
    // Check for invalid memory range or file descriptors
    if (check_mem((char *) buf, (int) count, SDRAM_START, SDRAM_END) == FALSE) {
        return EFAULT;
    // Again, if file descripter isn't stdout then we have an error
    } else if (fd != STDIN_FILENO) {
        return EBADF;
    }

    size_t i = 0;
    char *buffer = (char *) buf;
    char read_char;

    while (i < count) {
        read_char = getc();

        if (read_char == 4) { //EOT character
            return i;
        } else if (((read_char == 8) || (read_char == 127))) { // backspace or DEL character
            buffer[i] = 0; // '\0' character
            if(i > 0) {
                i--;
                puts("\b \b");
            }
        } else if ((read_char == 10) || (read_char == 13)) { // '\n' newline or '\r' carriage return character
            buffer[i] = '\n';
            putc('\n');
            return (i+1);
        } else {
            // put character into buffer and putc
            buffer[i] = read_char;
            i++;
            putc(read_char);
        }
    }

    return i;
}

/* Handle SWI's with appropriate action based on switch statement */
int C_SWI_Handler(int swiNum, int *regs) {
    // switch to appropriate swi call
    int count = 0;
    printf("I'm in C_SWI_Handler so my handler is probably wired in correctly \n");
    switch (swiNum) {
        // call read handler for read swi
        case READ_SWI:
            count = read_handler(regs[0], (void *) regs[1], (size_t) regs[2]);
            break;
        // Call write handler for write SWI
        case WRITE_SWI:
            count = write_handler((int) regs[0], (void *) regs[1], (size_t) regs[2]);
            break;
        // update time for time swi
        case TIME_SWI:
            count = os_time;
            break;
        // Call sleep for sleep swi
        case SLEEP_SWI:
            sleep((int) regs[0]);
            break; 
        case CREATE_SWI:
        	count = task_create((task_t*) regs[0], (size_t)regs[1]);
        	break;
        case MUTEX_CREATE:
        	count = mutex_create();
        	break;
    	case MUTEX_LOCK:
    		count = mutex_lock(regs[0]);
        	break;
		case MUTEX_UNLOCK:
			count = mutex_unlock(regs[0]);
        	break;
        case EVENT_WAIT:
        	count = event_wait((unsigned int) regs[0]);
        	break;
        default:
            printf("Error in ref C_SWI_Handler: Invalid SWI number.");
            invalid_syscall(swiNum); // never returns
    }

    return count;
}

void C_IRQ_Handler() {
    printf("in IRQ handler \n");
    printf("swi handler address: %x, irq handler address %x \n", *(int *)SWI_VECT_ADDR, *(int *)IRQ_VECT_ADDR);
    printf("swi handler address+1: %x, irq handler address+1 %x \n", *(int *)(SWI_VECT_ADDR+1), *(int *)(IRQ_VECT_ADDR+1));
    printf("os time is: %d, \n",os_time );

	int trigger, OSCR, OSSR;
	OSCR = reg_read(OSTMR_OSCR_ADDR); //grab oscr and ossr
	OSSR = reg_read(OSTMR_OSSR_ADDR);

	if (OSSR == 1) // Checking that the interupt came from M0 == OSCR
	{
		os_time += 10;
        // TODO: see if the frequency is the problem with the hanging
		trigger = OSCR + (OSTMR_FREQ); // Set next trigger up

		reg_write(OSTMR_OSMR_ADDR(0), trigger); 

		reg_set(OSTMR_OSSR_ADDR, OSTMR_OSSR_M0); // clear OSSR, clears interupt

	}
	dev_update(os_time);
	return;
}

void wire_in_handler(int vect_addr, int new_handler, int* old_handler, int* instr0, int* instr1) {
	printf("I'm in wire_in_handler \n");

    // Given the location of a vector_address and a new handler, 
	// wire in the new handler, and save the first two instructions
	// of the old handler
	// Jump offset already incorporates PC offset. Usually 0x10 or 0x14.
    int jmp_offset = (*((int *) vect_addr))&(0xFFF);

    // &S_Handler" in Jump Table.
    int *swi_handler_addr = *(int **)(vect_addr + PC_OFFSET + jmp_offset);
    printf("address of of old handler is %x \n", (int)(swi_handler_addr));
    // Save original Uboot SWI handler instructions.
    int swi_instr_1 = *swi_handler_addr;
    int swi_instr_2 = *(swi_handler_addr + 1);

    // Wire in our own: LDR pc, [pc, #-4] = 0xe51ff004
    *swi_handler_addr = LDR_PC_PC4_INSTR;
    *(swi_handler_addr + 1) = new_handler; // New swi handler.
    printf("address of handler is %x \n", new_handler);
    printf("address of of implanted handler is %x \n", *(swi_handler_addr + 1));

    *old_handler = (int) swi_handler_addr;
    *instr0 = swi_instr_1;
    *instr1 = swi_instr_2;
}

void push_args(int user_stack, int argc, char** argv, int* sp_top){
    // push args to the user stack in the right order
    int *spTop = ((int *) user_stack) - 1;
    int i = 0;
    // Push all members of argv onto stack
    for (i = argc-1; i >= 0; i--) {
        *spTop = (int)argv[i];
        spTop--;
    }
    // Push argc onto the stack
    *spTop = argc;

    *sp_top = (int) spTop;
}

int check_ldr_pc(int vect_addr) {
    int vector_instr = *((int *)vect_addr);

    // Check if the offset is negative.
    if ((vector_instr & LDR_SIGN_MASK) == 0) {
        return 0;
    }

    // Check that the instruction is a (LDR pc, [pc, 0x000])
    if ((vector_instr & 0xFFFFF000) != LDR_PC_PC_INSTR) {
        return 0;
    }

    return 1;
}

int kmain(int argc __attribute__((unused)), char** argv  __attribute__((unused)), uint32_t table)
{
    printf("Starting kmain up \n");
    printf("swi handler address: %x, irq handler address %x \n", *(int *)SWI_VECT_ADDR, *(int *)IRQ_VECT_ADDR);
    printf("swi handler address+1: %x, irq handler address+1 %x \n", *(int *)(SWI_VECT_ADDR+1), *(int *)(IRQ_VECT_ADDR+1));

    app_startup();
	global_data = table;
	/* add your code up to assert statement */

	int swi_instr0, swi_instr1, irq_instr0, irq_instr1, old_swi_handler, sp_top, old_irq_handler;
	// Initialize these so they're assigned locations in memory
	swi_instr0 = 0;
	swi_instr1 = 0;
	old_swi_handler = 0;
	old_irq_handler = 0;
	sp_top = 0;

    if (!check_ldr_pc(SWI_VECT_ADDR) && !check_ldr_pc(IRQ_VECT_ADDR)){
        printf("Instruction at vector address wrong");
        return 0xBADC0DE;
    }

    printf("starting wiring in of SWI and IRQ handlers \n");
    // Install our SWI and IRQ handlers

    printf("Address of irq_wrapper is: %x, Address of swi_handler is: %x \n", (int)&irq_wrapper, (int)&swi_handler);
    wire_in_handler(IRQ_VECT_ADDR, (int) &irq_wrapper, &old_irq_handler, &irq_instr0, &irq_instr1);
    wire_in_handler(SWI_VECT_ADDR, (int) &swi_handler, &old_swi_handler, &swi_instr0, &swi_instr1);
    printf("old_swi_handler: %x swi_instr0: %x, swi_instr1: %x\n", old_swi_handler, swi_instr0, swi_instr1);    // Set up the stack
    
    printf("pushing args\n");
    push_args(USER_STACK_TOP, argc, argv, &sp_top);
    printf("sp_top:%x\n",sp_top);

	// Init timer and allow it to trigger interrupts
	reg_write(OSTMR_OSSR_ADDR, 0x0); // clear status reg
	reg_write(OSTMR_OSMR_ADDR(0), OSTMR_FREQ / 100); // set first trigger
	reg_write(INT_ICMR_ADDR, 1 << INT_OSTMR_0); // mask for interrupts
	reg_write(INT_ICLR_ADDR, 0x0);	// turn on interrupts
	reg_write(OSTMR_OIER_ADDR, OSTMR_OIER_E0); // setup M0
	reg_write(OSTMR_OSCR_ADDR, 0x0); // set count to zero




    irq_setup();
    printf("switching to user\n");
    return user_setup((int *)sp_top);
    printf("Shouldn't get here\n");

	assert(0);        /* should never get here */
}

