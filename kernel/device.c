/**
 * @file device.c
 *
 * @brief Implements simulated devices.
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-12-01
 */

#include <types.h>
#include <assert.h>

#include <task.h>
#include <sched.h>
#include <device.h>
#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>

/**
 * @brief Fake device maintainence structure.
 * Since our tasks are periodic, we can represent 
 * tasks with logical devices. 
 * These logical devices should be signalled periodically 
 * so that you can instantiate a new job every time period.
 * Devices are signaled by calling dev_update 
 * on every timer interrupt. In dev_update check if it is 
 * time to create a tasks new job. If so, make the task runnable.
 * There is a wait queue for every device which contains the tcbs of
 * all tasks waiting on the device event to occur.
 */

struct dev
{
	tcb_t* sleep_queue;
	unsigned long   next_match;
};
typedef struct dev dev_t;

/* devices will be periodically signaled at the following frequencies */
const unsigned long dev_freq[NUM_DEVICES] = {100, 200, 500, 50};
static dev_t devices[NUM_DEVICES];

extern unsigned long volatile os_time;

/**
 * @brief Initialize the sleep queues and match values for all devices.
 */
void dev_init(void)
{
	int i;
	for (i = 0; i < NUM_DEVICES; i++) {
		devices[i].sleep_queue=0; // Set each sleep queue to iniially be empty
		devices[i].next_match=dev_freq[i]; // Set each next_match to be the appropriate
		// Time from dev_freq
	}

}


/**
 * @brief Puts a task to sleep on the sleep queue until the next
 * event is signalled for the device.
 *
 * @param dev  Device number.
 */
void dev_wait(unsigned int dev __attribute__((unused)))
{
	// Get the task on the top of the sleep queue
	tcb_t* queueTask = devices[dev].sleep_queue;
	// get the current task
	tcb_t* curTask = get_cur_tcb();

	// Add the current task to the sleep queue
	// loop through sleep_queue until you hit the end
	while (queueTask->sleep_queue != 0) { // Hits end when sleep_queue == 0
		queueTask = queueTask->sleep_queue;
	}
	// Set end of sleep queue to point to current task
	queueTask->sleep_queue = curTask;
	// Make sure sleep q pointer of current task is at device's sleep queue
	curTask->sleep_queue = devices[dev].sleep_queue;
}


/**
 * @brief Signals the occurrence of an event on all applicable devices. 
 * This function should be called on timer interrupts to determine that 
 * the interrupt corresponds to the event frequency of a device. If the 
 * interrupt corresponded to the interrupt frequency of a device, this 
 * function should ensure that the task is made ready to run 
 */
void dev_update(unsigned long millis __attribute__((unused)))
{
	tcb_t* queueTask;
	tcb_t* lastTask;
	// Update all sleep queues
	int i;
	
	for (i = 0; i < NUM_DEVICES; ++i) 
	{
		// If time has passed to update device
		if (devices[i].next_match <= millis) {
			// Loop through sleep queue
			queueTask = devices[i].sleep_queue;
			while (queueTask != 0) {
				// Add each task on sleepqueue to runqueue
				runqueue_add(queueTask, queueTask->cur_prio);

				lastTask = queueTask;
				// Iterate to next task in sleep queue
				queueTask = queueTask->sleep_queue;
				// clear sleep_queue value for tasks leaving sleep queue
				lastTask->sleep_queue = 0;
			}
		// Update next match for the next match
		devices[i].next_match = os_time + dev_freq[i];
		}
	}
}

