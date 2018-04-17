// os345semaphores.c - OS Semaphores
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the BYU CS345 projects.      **
// ** It comes "as is" and "unwarranted."  As such, when you use part   **
// ** or all of the code, it becomes "yours" and you are responsible to **
// ** understand any algorithm or method presented.  Likewise, any      **
// ** errors or problems become your responsibility to fix.             **
// **                                                                   **
// ** NOTES:                                                            **
// ** -Comments beginning with "// ??" may require some implementation. **
// ** -Tab stops are set at every 3 spaces.                             **
// ** -The function API's in "OS345.h" should not be altered.           **
// **                                                                   **
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// ***********************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>

#include "os345.h"


extern TCB tcb[];							// task control block
extern struct pqueue rq;
extern int curTask;							// current task #

extern int superMode;						// system mode
extern Semaphore* semaphoreList;			// linked list of active semaphores


// **********************************************************************
// **********************************************************************
// signal semaphore
//
//	if task blocked by semaphore, then clear semaphore and wakeup task
//	else signal semaphore
//
void semSignal(Semaphore* s)
{
	// assert there is a semaphore and it is a legal type
	assert("semSignal Error" && s);
	assert("semSignal Error" && ((s->type == BINARY) || (s->type == COUNTING)));
	// check semaphore type
	if (s->type == 0)
	{
		// binary semaphore
		// look through tasks for one suspended on this semaphore

		struct pqElement task;

		task = dequeue(s->wq, -1);
		if (task.tid != -1)
		{
			s->state = 0;				// clear semaphore
			tcb[task.tid].event = 0;			// clear event pointer
			tcb[task.tid].state = S_READY;	// unblock task

			// ?? move task from blocked to ready queue
			//printf("\nsemSignal: %d, %s\n", task.tid, s->name);
			enqueue(&rq, task);

			if (!superMode) swapTask();
			return;

		}
		// nothing waiting on semaphore, go ahead and just signal
		s->state = 1;						// nothing waiting, signal
		if (!superMode) swapTask();
		return;
	}
	else
	{
		// counting semaphore
		// ?? implement counting semaphore
		struct pqElement task;

		task = dequeue(s->wq, -1);
		if (task.tid != -1)
		{
			s->state++;				// clear semaphore
			tcb[task.tid].event = 0;			// clear event pointer
			tcb[task.tid].state = S_READY;	// unblock task

			// ?? move task from blocked to ready queue
			//printf("\nsemSignal: %d, %s\n", task.tid, s->name);
			enqueue(&rq, task);

			if (!superMode) swapTask();
			return;

		}
		// nothing waiting on semaphore, go ahead and just signal
		s->state++;						// nothing waiting, signal
		if (!superMode) swapTask();
		return;
	}
} // end semSignal



// **********************************************************************
// **********************************************************************
// wait on semaphore
//
//	if semaphore is signaled, return immediately
//	else block task
//
int semWait(Semaphore* s)
{
	assert("semWait Error" && s);												// assert semaphore
	assert("semWait Error" && ((s->type == BINARY) || (s->type == COUNTING)));	// assert legal type
	assert("semWait Error" && !superMode);								// assert user mode

	// check semaphore type
	if (s->type == 0)
	{
		// binary semaphore
		// if state is zero, then block task

		if (s->state == 0)
		{
			tcb[curTask].event = s;		// block task
			tcb[curTask].state = S_BLOCKED;
			struct pqElement task;
			// ?? move task from ready queue to blocked queue
			//printf("\nsemWait: %d, %s\n", curTask, s->name);


			task.tid = curTask;
			task.priority = tcb[curTask].priority;
			dequeue(&rq, curTask);
			enqueue(s->wq, task);

			swapTask();						// reschedule the tasks
			return 1;
		}
		// state is non-zero (semaphore already signaled)
		s->state = 0;						// reset state, and don't block
		return 0;
	}
	else
	{
		// counting semaphore
		// ?? implement counting semaphore
		if (s->state <= 0)
		{
			tcb[curTask].event = s;		// block task
			tcb[curTask].state = S_BLOCKED;
			struct pqElement task;
			// ?? move task from ready queue to blocked queue
			//printf("\nsemWait: %d, %s\n", curTask, s->name);
			s->state--;

			task.tid = curTask;
			task.priority = tcb[curTask].priority;
			dequeue(&rq, curTask);
			enqueue(s->wq, task);

			swapTask();						// reschedule the tasks
			return 1;
		}
		// state is non-zero (semaphore already signaled)
		s->state--;						// reset state, and don't block
		return 0;

	}
} // end semWait



// **********************************************************************
// **********************************************************************
// try to wait on semaphore
//
//	if semaphore is signaled, return 1
//	else return 0
//
int semTryLock(Semaphore* s)
{
	assert("semTryLock Error" && s);												// assert semaphore
	assert("semTryLock Error" && ((s->type == 0) || (s->type == 1)));	// assert legal type
	assert("semTryLock Error" && !superMode);									// assert user mode

	// check semaphore type
	if (s->type == 0)
	{
		// binary semaphore
		// if state is zero, then block task

temp:	// ?? temporary label
		if (s->state == 0)
		{
			return 0;
		}
		// state is non-zero (semaphore already signaled)
		s->state = 0;						// reset state, and don't block
		return 1;
	}
	else
	{
		// counting semaphore
		// ?? implement counting semaphore

		goto temp;
	}
} // end semTryLock


// **********************************************************************
// **********************************************************************
// Create a new semaphore.
// Use heap memory (malloc) and link into semaphore list (Semaphores)
// 	name = semaphore name
//		type = binary (0), counting (1)
//		state = initial semaphore state
// Note: memory must be released when the OS exits.
//
Semaphore* createSemaphore(char* name, int type, int state)
{
	Semaphore* sem = semaphoreList;
	Semaphore** semLink = &semaphoreList;

	// assert semaphore is binary or counting
	assert("createSemaphore Error" && ((type == 0) || (type == 1)));	// assert type is validate

	// look for duplicate name
	while (sem)
	{
		if (!strcmp(sem->name, name))
		{
			printf("\nSemaphore %s already defined", sem->name);

			// ?? What should be done about duplicate semaphores ??
			// semaphore found - change to new state
			sem->type = type;					// 0=binary, 1=counting
			sem->state = state;				// initial semaphore state
			sem->taskNum = curTask;			// set parent task #
			return sem;
		}
		// move to next semaphore
		semLink = (Semaphore**)&sem->semLink;
		sem = (Semaphore*)sem->semLink;
	}

	// allocate memory for new semaphore
	sem = (Semaphore*)malloc(sizeof(Semaphore));

	// set semaphore values
	sem->name = (char*)malloc(strlen(name)+1);
	strcpy(sem->name, name);				// semaphore name
	sem->type = type;							// 0=binary, 1=counting
	sem->state = state;						// initial semaphore state
	sem->taskNum = curTask;					// set parent task #

	//**********************CREATING WAITING QUEUE**********************
	sem->wq = (struct pqueue*)malloc(sizeof(struct pqueue));
	(sem->wq)->data = (struct pqElement*)malloc(MAX_TASKS * sizeof(struct pqElement));
	initialize(sem->wq);

	//******************************************************************

	// prepend to semaphore list
	sem->semLink = (struct semaphore*)semaphoreList;
	semaphoreList = sem;						// link into semaphore list
	return sem;									// return semaphore pointer
} // end createSemaphore



// **********************************************************************
// **********************************************************************
// Delete semaphore and free its resources
//
bool deleteSemaphore(Semaphore** semaphore)
{
	Semaphore* sem = semaphoreList;
	Semaphore** semLink = &semaphoreList;

	// assert there is a semaphore
	assert("deleteSemaphore Error" && *semaphore);

	// look for semaphore
	while(sem)
	{
		if (sem == *semaphore)
		{
			// semaphore found, delete from list, release memory
			*semLink = (Semaphore*)sem->semLink;

			// free the name array before freeing semaphore
			printf("\ndeleteSemaphore(%s)", sem->name);

			// ?? free all semaphore memory
			// ?? What should you do if there are tasks in this
			//    semaphores blocked queue????
			int tid;
			while((tid=(dequeue((sem->wq), -1)).tid) != -1)
			{

				printf("\nkilling task: %d", tid);
				killTask(tid);
			}
			free((sem->wq)->data);
			free(sem->wq);

			free(sem->name);
			free(sem);

			return TRUE;
		}
		// move to next semaphore
		semLink = (Semaphore**)&sem->semLink;
		sem = (Semaphore*)sem->semLink;
	}

	// could not delete
	return FALSE;
} // end deleteSemaphore
