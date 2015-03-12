/** @file ub_test.c
 * 
 * @brief The UB Test for basic schedulability
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-20
 */

//#define DEBUG 0

#include <sched.h>
#include <math.h>
#ifdef DEBUG
#include <exports.h>
#endif
/* Simple insertion for tasks sorting by priorty */ 
void insertion_sort(task_t** tasks, size_t num_tasks){
	size_t i, j;
	task_t* t;
	// Find elements sequentially 
	for (i = 0; i < num_tasks; i++)
	{
		t = tasks[i];
		j = i;
		// Organize tasks by priority
		while(j > 0 && tasks[j-1]->T > t->T)
		{
			tasks[j] = tasks[j-1];
			j = j - 1;
		}
		tasks[j] = t;
	}
}


/**
 * @brief Perform UB Test and reorder the task list.
 *
 * The task list at the end of this method will be sorted in order is priority
 * -- from highest priority (lowest priority number) to lowest priority
 * (highest priority number).
 *
 * @param tasks  An array of task pointers containing the task set to schedule.
 * @param num_tasks  The number of tasks in the array.
 *
 * @return 0  The test failed.
 * @return 1  Test succeeded.  The tasks are now in order.
 */
 // Function which tests schedulability of a task set using the ub admissibility rule
int assign_schedule(task_t** tasks  __attribute__((unused)), size_t num_tasks  __attribute__((unused)))
{
	// Sort the tasks
	insertion_sort(tasks, num_tasks);
	size_t i;
	double total_time, uk;
	total_time = 0;
	uk = (double) num_tasks * (ilog2((double)(2 * num_tasks)) - 1.);
	// Total time is the summation of case execution time / periodicity for each task
	for (i = 0; i < num_tasks; i++)
	{
		total_time += (double) tasks[i]->C/ (double)tasks[i]->T;
	}

	return uk >= total_time + (double) tasks[i]->B / (double) tasks[i]->T; // fAdd worst-case blocking of the last task	
}
	


