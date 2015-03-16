/** @file ctx_switch.c
 * 
 * @brief C wrappers around assembly context switch routines.
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-21
 */
 

#include <types.h>
#include <assert.h>

#include <config.h>
#include <kernel.h>
#include "sched_i.h"

#include "arm/exception.h"

#ifdef DEBUG_MUTEX
#include <exports.h>
#endif

static __attribute__((unused)) tcb_t* cur_tcb; /* use this if needed */

/**
 * @brief Initialize the current TCB and priority.
 *
 * Set the initialization thread's priority to IDLE so that anything
 * will preempt it when dispatching the first task.
 */
void dispatch_init(tcb_t* idle __attribute__((unused)))
{
	// initially just idle, so set idle as the current task
	cur_tcb = idle;
	// Add the idle task to the run queue
	runqueue_add(idle, IDLE_PRIO);
}


/**
 * @brief Context switch to the highest priority task while saving off the 
 * current task state.
 *
 * This function needs to be externally synchronized.
 * We could be switching from the idle task.  The priority searcher has been tuned
 * to return IDLE_PRIO for a completely empty run_queue case.
 */
void dispatch_save(void)
{
	disable_interrupts();
	// Save current task to runqueue unless it's the idle task
	// If it's the idle task then it's already permanently in the runqueue
	if (cur_tcb->cur_prio != IDLE_PRIO) {
		runqueue_add(cur_tcb, cur_tcb->cur_prio);
	}

	// get the highest priority task
	tcb_t* new_tcb = runqueue_remove(highest_prio());

	// Context switch to new task
	ctx_switch_full(&(new_tcb->context), &(cur_tcb->context));
	cur_tcb = new_tcb;
	enable_interrupts();
}

/**
 * @brief Context switch to the highest priority task that is not this task -- 
 * don't save the current task state.
 *
 * There is always an idle task to switch to.
 */
void dispatch_nosave(void)
{	
	printf("Into dispatch_nosave \n");
	disable_interrupts();
	// get the highest priority task
	printf("Removing task from runqueue \n");
	tcb_t* new_tcb = runqueue_remove(highest_prio());
	printf("Task now removed, half ctx switching to new task \n");
	printf("tcb info: \n");
	printf("&new_tcb->context = %x\n", &new_tcb->context);
	printf("((&(new_tcb->context.r4))) = %x\n",((&(new_tcb->context.r4))));
	printf("((&(new_tcb->context.r5))) = %x\n",((&(new_tcb->context.r5))));
	printf("((&(new_tcb->context.r6))) = %x\n",((&(new_tcb->context.r6))));
	printf("((&(new_tcb->context.r7))) = %x\n",((&(new_tcb->context.r7))));
	printf("((&(new_tcb->context.r8))) = %x\n",((&(new_tcb->context.r8))));
	printf("((&(new_tcb->context.r9))) = %x\n",((&(new_tcb->context.r9))));
	printf("((&(new_tcb->context.r10))) = %x\n",((&(new_tcb->context.r10))));
	printf("((&(new_tcb->context.r11))) = %x\n",((&(new_tcb->context.r11))));

	printf("((&(new_tcb->context.sp))) = %x\n",((&(new_tcb->context.sp))));
	printf("((&(new_tcb->context.lr))) = %x\n",((&(new_tcb->context.lr))));

	printf("*(int*)((&(new_tcb->context))) = %x\n", *(int*)((&(new_tcb->context)))); 
	printf("new_tcb->context.sp = %x\n", new_tcb->context.sp); 
	printf("new_tcb->context.lr = %x\n", new_tcb->context.lr);
	printf("new_tcb->context.r4 = %x\n", new_tcb->context.r4);
	printf("new_tcb->context.r5 = %x\n", new_tcb->context.r5);
	printf("new_tcb->context.r6 = %x\n", new_tcb->context.r6);
	printf("new_tcb->context.r7 = %x\n", new_tcb->context.r7);
	printf("new_tcb->context.r8 = %x\n", new_tcb->context.r8);
	printf("new_tcb->context.r9 = %x\n", new_tcb->context.r9);

	printf("launch task location is = %x\n", launch_task);


	// Context switch to new task
	ctx_switch_half(&(new_tcb->context));
	printf("Returned from half context switch in dispatch nosave \n");
	cur_tcb = new_tcb;
	enable_interrupts();
}


/**
 * @brief Context switch to the highest priority task that is not this task -- 
 * and save the current task but don't mark is runnable.
 *
 * There is always an idle task to switch to.
 */
void dispatch_sleep(void)
{
	disable_interrupts();
	// get the highest priority task
	tcb_t* new_tcb = runqueue_remove(highest_prio());
	// IF highest priority task is current task, 
	// remove the next highest task
	if (new_tcb == cur_tcb) {
		new_tcb = runqueue_remove(highest_prio());
	}
	// Context switch to new task
	ctx_switch_full(&(new_tcb->context), &(cur_tcb->context));
	cur_tcb = new_tcb;
	enable_interrupts();	
}

/**
 * @brief Returns the priority value of the current task.
 */
uint8_t get_cur_prio(void)
{
	return cur_tcb->cur_prio; //simply return priority of current task
}

/**
 * @brief Returns the TCB of the current task.
 */
tcb_t* get_cur_tcb(void)
{
	return cur_tcb; //simply return cur_tcb
}

void print(int * r0){
	printf("address of r0 is %x \n", (int)r0);
	printf("Contents are %x\n",*r0);
}