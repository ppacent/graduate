/** @file sched.c
 * 
 * @brief Top level implementation of the scheduler.
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-20
 */

#include <types.h>
#include <assert.h>

#include <kernel.h>
#include <config.h>
#include "sched_i.h"

#include "lock.h"
#include "device.h"

#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/physmem.h>

tcb_t system_tcb[OS_MAX_TASKS]; /*allocate memory for system TCBs */
uint32_t idle_stack[OS_KSTACK_SIZE]; // allocate idle stack

/* To initialize, initialize mutexes, runqueue, and device */
void sched_init(task_t* main_task  __attribute__((unused)))
{
	mutex_init();
	runqueue_init();
	dev_init();
}

/**
 * @brief This is the idle task that the system runs when no other task is runnable
 */
 
static void __attribute__((unused)) idle(void)
{
	 enable_interrupts();
	 while(1);
}

/**
 * @brief Allocate user-stacks and initializes the kernel contexts of the
 * given threads.
 *
 * This function assumes that:
 * - num_tasks < number of tasks allowed on the system.
 * - the tasks have already been deemed schedulable and have been appropriately
 *   scheduled.  In particular, this means that the task list is sorted in order
 *   of priority -- higher priority tasks come first.
 *
 * @param tasks  A list of scheduled task descriptors.
 * @param size   The number of tasks is the list.
 */
void allocate_tasks(task_t** tasks  __attribute__((unused)), size_t num_tasks  __attribute__((unused)))
{

	// Set up runqueue, mutex, and dev
	sched_init(tasks[0]); // Task is unused in function
	task_t* task_list = *tasks;

	/* Set up the system tcb */
	// Add all the tasks to the system_tcb array with the data in the appropriate struct format
	size_t i;


	/* For some reason, task_list[1] contains 0th task */
	for (i = 1; i < num_tasks+1; i++)
	{
		system_tcb[i].native_prio = i;
		system_tcb[i].cur_prio = i;

		// Set up registers with appropriate values
		printf("\nPrinting task info in for loop...\n");
		printf("Lambda is: %x, data is: %x, stack position is: %x\n",(uint32_t)task_list[i].lambda,
		(uint32_t)task_list[i].data, (uint32_t)task_list[i].stack_pos );
		printf("sp is: %x, lr/launch_task is %x \n\n",system_tcb[i].kstack_high,launch_task);

		system_tcb[i].context.r4 = (uint32_t) task_list[i].lambda;
		system_tcb[i].context.r5 = (uint32_t) task_list[i].data;
		system_tcb[i].context.r6 = (uint32_t) task_list[i].stack_pos;
		system_tcb[i].context.sp = system_tcb[i].kstack_high; //TODO: double check that (sp twice) is what I want
		system_tcb[i].context.lr = launch_task;

		// Initially neither sleeping nor locking
		system_tcb[i].holds_lock = 0; 
		system_tcb[i].sleep_queue = 0;
		
		// Set kernel stack to appropriate area 
		//system_tcb[i].kstack[0] = (uint32_t) tasks[i]->stack_pos;
		//system_tcb[i].kstack_high[0] = (uint32_t) tasks[i]->stack_pos + OS_KSTACK_SIZE/sizeof(uint32_t);

		// Add task to the run queue
		runqueue_add(&system_tcb[i], system_tcb[i].cur_prio);
	}

	// Create Idle task
	system_tcb[IDLE_PRIO].native_prio = IDLE_PRIO;
	system_tcb[IDLE_PRIO].cur_prio = IDLE_PRIO;

	system_tcb[IDLE_PRIO].context.r4 = (int)&idle;
	system_tcb[IDLE_PRIO].context.r5 = 0;
	system_tcb[IDLE_PRIO].context.r6 = (int)system_tcb[IDLE_PRIO].kstack;
	system_tcb[IDLE_PRIO].context.sp = system_tcb[IDLE_PRIO].kstack_high;
	system_tcb[IDLE_PRIO].context.lr = launch_task;

	system_tcb[IDLE_PRIO].holds_lock = 0;
	system_tcb[IDLE_PRIO].sleep_queue = 0;
	
	//system_tcb[IDLE_PRIO].kstack[0] = (int)&idle_stack;
	//system_tcb[IDLE_PRIO].kstack_high[0] = (int)&idle_stack + OS_KSTACK_SIZE/sizeof(uint32_t);

	runqueue_add(&system_tcb[IDLE_PRIO], IDLE_PRIO);


	printf("Tasks allocated, now going to dispatch \n");
	// start running highest priority task
	dispatch_nosave();
}

