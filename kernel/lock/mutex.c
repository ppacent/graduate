/**
 * @file mutex.c
 *
 * @brief Implements mutices.
 *
 * @author Harry Q Bovik < PUT YOUR NAMES HERE
 *
 * 
 * @date  
 */

//#define DEBUG_MUTEX

#include <lock.h>
#include <task.h>
#include <sched.h>
#include <bits/errno.h>
#include <arm/psr.h>
#include <arm/exception.h>
#ifdef DEBUG_MUTEX
#include <exports.h> // temp
#endif

mutex_t gtMutex[OS_NUM_MUTEX];

/* Set up all mutexes to be initially unavailable, unlocked, 
and with empty sleep and holding queues */
void mutex_init()
{
	// Set all mutexes to be unassigned
	int i;
	for (i = 0; i < OS_NUM_MUTEX; i++) { 

		gtMutex[i].bAvailable = 0;		/* Not available at mutex init */
		gtMutex[i].pHolding_Tcb = 0;	/* Nothing is  using this mutex upon init */
		gtMutex[i].bLock = 0;			/* All mutexes unlocked upon init */	
		gtMutex[i].pSleep_queue = 0; 	/* Nothing is waiting for this mutex upon init */
	}
}

/* Create a mutex by making it available, and return the identifier */
int mutex_create(void)
{
	disable_interrupts();
	int i;
	// Find an unavailable mutex and make it available
	for (i = 0; i < OS_NUM_MUTEX; i++)
	{
		// Once we have find unavailble mutex and have made it available,return
		if (gtMutex[i].bAvailable == 0)
		{
			gtMutex[i].bAvailable = 1;
			enable_interrupts();
			return i;
		}
	}	
	// If we exit the loop, all mutex's are created 
	enable_interrupts();
	return ENOMEM; // fix this to return the correct value
}

/* Lock the mutex with identifier int mutex */ 
int mutex_lock(int mutex  __attribute__((unused)))
{

	disable_interrupts();
	tcb_t* curTask = gtMutex[mutex].pHolding_Tcb;

	if (gtMutex[mutex].bAvailable == 0)
		// block is not available, return error
	{
		enable_interrupts();
		return EINVAL;
	}

	if (gtMutex[mutex].bLock == 0)
	// mutex is available, lock it, and assign current task to it
	{
		gtMutex[mutex].bLock = 1;
		gtMutex[mutex].pHolding_Tcb = get_cur_tcb();
		curTask->holds_lock = 1;
		runqueue_add(get_cur_tcb(), 0); // run at highest priority

	}
	else
	// mutex is blocked, add the current task to the sleep queue
	{
		if (gtMutex[mutex].pHolding_Tcb == get_cur_tcb())
			// If the current tasks already holds the mutex we're in deadlock
		{
			enable_interrupts();
			return EDEADLOCK;
		}
		tcb_t* task = gtMutex[mutex].pSleep_queue;
		while(task != 0)
			// append the task to the end of the sleep queue
		{
			task = task->sleep_queue;
		}
		task->sleep_queue = get_cur_tcb();
		runqueue_remove(get_cur_tcb()->cur_prio);
	}
	enable_interrupts();
	return 0; 
}

int mutex_unlock(int mutex  __attribute__((unused)))
{
	disable_interrupts();
	tcb_t* curTask = gtMutex[mutex].pHolding_Tcb;
	if (gtMutex[mutex].bAvailable == 0)
		// block is not available, return error
	{
		enable_interrupts();
		return EINVAL;
	}

	if (gtMutex[mutex].bLock == 1)
	{
		if (gtMutex[mutex].pHolding_Tcb == get_cur_tcb())
		{
			if (gtMutex[mutex].pSleep_queue == 0)
			// if the sleep queue is empty, unlock the mutex, 
			// take the task off the holding queue, and remove the update the tcb
			{
				gtMutex[mutex].bLock = 0; // Mutex is no longer blocking
				gtMutex[mutex].pHolding_Tcb = 0; // Mutex no longer owns task
			}
			else 
				// Move to the next task in the sleep queue
			{
				gtMutex[mutex].pHolding_Tcb = gtMutex[mutex].pSleep_queue;
				gtMutex[mutex].pSleep_queue = gtMutex[mutex].pSleep_queue->sleep_queue;
				runqueue_add(gtMutex[mutex].pHolding_Tcb, 0); // Set to the highest priority
			}
			curTask->holds_lock = 0; // Task no longer holds the lock
		}
		else 
			// Mutex is not held by current task, return error
		{
			enable_interrupts();
			return EPERM;
		}

	}
	else
	{
		enable_interrupts();
		return EPERM;
	}
	
	enable_interrupts();
	return 0; 
}


