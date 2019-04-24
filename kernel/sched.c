#include <kernel/task.h>
#include <inc/x86.h>

#define ctx_switch(ts) \
  do { env_pop_tf(&((ts)->tf)); } while(0)

/* TODO: Lab5
* Implement a simple round-robin scheduler (Start with the next one)
*
* 1. You have to remember the task you picked last time.
*
* 2. If the next task is in TASK_RUNNABLE state, choose
*    it.
*
* 3. After your choice set cur_task to the picked task
*    and set its state, remind_ticks, and change page
*    directory to its pgdir.
*
* 4. CONTEXT SWITCH, leverage the macro ctx_switch(ts)
*    Please make sure you understand the mechanism.
*/
void sched_yield(void)
{
	extern Task tasks[];
	extern Task *cur_task;
	
	int cur_task_id = cur_task->task_id;
	int picked_task_id = cur_task_id;
	int task_it;
	for(task_it = 0; task_it < NR_TASKS ; task_it++)
	{
		int task_id = (cur_task_id + task_it) % NR_TASKS;
		if (tasks[task_id].state == TASK_RUNNABLE)
		{
			picked_task_id = task_id;
			break;
		}
	}

	cur_task = &tasks[picked_task_id];
	cur_task->remind_ticks = TIME_QUANT;
	cur_task->state = TASK_RUNNING;

	// load page directory
	lcr3(PADDR(cur_task->pgdir));
	// context switch
	ctx_switch(cur_task);
}
