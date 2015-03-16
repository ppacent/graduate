/** @file proc.c
 * 
 * @brief Implementation of `process' syscalls
 *
 * @author Peter Pacent ppacent@andrew.cmu.edu
 */

#include <exports.h>
#include <bits/errno.h>
#include <config.h>
#include <kernel.h>
#include <syscall.h>
#include <sched.h>

#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/physmem.h>
#include <device.h>



/* Order a task array according to rate monotonic scheduling for a given number 
of tasks */
void schedule_tasks_rms(task_t* tasks, size_t num_tasks) {
  printf("!:!Inside Scheduling tasks \n");
  /* Not trying to get to fancy, just doing O(n^2) sort for now */
  int i = 0;
  int j = 0;
  int next_prio_i = 0;
  task_t next_prio_task;
  printf("Entering for loop \n");
  for (i = 0; i < (int) num_tasks; i++) {
    printf("In Outer loop \n");


    next_prio_i = i; // Looking for the ith priority tasks
    next_prio_task = tasks[i]; // inititially set task to first available task

    //printf("\nPrinting task info in for schedule_tasks loop...\n");
    //printf("Lambda is: %x, data is: %x, stack position is: %x\n",(uint32_t)(next_prio_task.lambda),
    //(uint32_t)(next_prio_task.data), (uint32_t)(next_prio_task.stack_pos ));
    for (j = i; j < (int) num_tasks; j++) {
      printf("In inner for loop \n");

      // If we find a task with a lower period, it's in line to 
      // be the next highest priority
      if (tasks[j].T < tasks[next_prio_i].T) {
        next_prio_i = j;
      }
    }
    // Rearrange tasks for next highest priority
    next_prio_task = tasks[next_prio_i]; // store next highest task

    tasks[next_prio_i] = tasks[j]; // move task at next index to old spot of 
    // the next highest priority task

    tasks[j] = next_prio_task;
  } // Rinse and repeat

}
/* Create tasks by kernel. Once this is called you can assume that all
pre-existing tasks are hereafter ignored by the kernel. Get information
about given tasks from the task_t's that are passed in. Verify that input is correct
from user (it is not guaranteed to be accurate) */
int task_create(task_t* tasks  __attribute__((unused)), size_t num_tasks  __attribute__((unused)))
{
  // Schedule tasks prior to allocating based on scheduling algorithm
  //size_t i;
  //printf("Size of task structure is %d \n", sizeof(task_t));
  //task_t* kernel_tasks = malloc(num_tasks*sizeof(task_t));
  /* This whole define tasks two different ways thing is probably the most frustrating bug ever*/
  // Port the task array from the user format to the kernel format
  /*for (i = 0; i < num_tasks; i++) {
    printf("\nTask[%d] address, is located at %x \n", i, &tasks[i]);
    printf("Task[%d] lambda, %x, is located at %x \n", i, tasks[i].lambda, &(tasks[i].lambda));
    printf("Task[%d] data, %x, is located at %x \n", i, tasks[i].data, &(tasks[i].data));
    printf("Task[%d] stack_pos, %x, is located at %x \n", i, tasks[i].stack_pos, &(tasks[i].stack_pos));
    printf("Task[%d] C, %x, is located at %x \n", i, tasks[i].C, &(tasks[i].C));
    printf("Task[%d] T, %x, is located at %x \n", i, tasks[i].T, &(tasks[i].T));
    printf("Task[%d] B, %x, is located at %x \n", i, tasks[i].B, &(tasks[i].B));
    /*
    kernel_tasks[i].lambda = *(task_fun_t*)(((uint32_t)tasks[i].lambda)- (4 * i)); // account for 4 bytes fewer that user stack has
    kernel_tasks[i].data = *(void**)(((uint32_t)tasks[i].lambda) - (4 * i) + 4);
    kernel_tasks[i].stack_pos = *(void**)(((uint32_t)tasks[i].lambda) - (4 * i) + 8);
    kernel_tasks[i].C = *(int*)(((uint32_t)tasks[i].lambda) - (4 * i) + 12);
    kernel_tasks[i].T = *(int*)(((uint32_t)tasks[i].lambda) - (4 * i) + 16);
    
    }*/
  

  disable_interrupts();

  if (num_tasks > OS_AVAIL_TASKS)
  {
  	return EINVAL;
  }

  schedule_tasks_rms(tasks, num_tasks); // Schedule according to rate monotonic scheduling
  allocate_tasks(&tasks, num_tasks);

  return 0;
}

/* Wait for event 
TODO: Verify that this works */
int event_wait(unsigned int dev  __attribute__((unused)))
{
  // Disable interrupts while waiting
  disable_interrupts();
	dev_wait(dev);
  enable_interrupts();
  // Re enable interrupts after done
  return 1; 	
}

/* An invalid syscall causes the kernel to exit. */
void invalid_syscall(unsigned int call_num  __attribute__((unused)))
{
	printf("Kernel panic: invalid syscall -- 0x%08x\n", call_num);

	disable_interrupts();
	while(1);
}
