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
	cur_tcb = idle;
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
	// get the highest priority task
	tcb_t* new_task = runqueue_remove(highest_prio());

	// Add task to the run_queue
	runqueue_add(new_task, new_task->cur_prio);

	// Context switch to new task
	ctx_switch_full(&(new_task->context), &(cur_tcb->context));
	cur_tcb = new_task;
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
	disable_interrupts();
	// get the highest priority task
	tcb_t* new_task = runqueue_remove(highest_prio());

	// Add task to the run_queue
	runqueue_add(new_task, new_task->cur_prio);

	// Context switch to new task
	ctx_switch_half(&(new_task->context));
	cur_tcb = new_task;
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
	runqueue_remove(cur_tcb->cur_prio);
	tcb_t* new_task = runqueue_remove(highest_prio());

	// Context switch to new task
	ctx_switch_full(&(new_task->context), &(cur_tcb->context));
	cur_tcb = new_task;
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
