#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdbool.h>
#include "thread.h"
#include "interrupt.h"

void thread_stub(void (*thread_main)(void *), void *arg);

/* This is the thread control block */
struct thread {
	Tid thread_id;
	struct ucontext* thread_ucontext;
};typedef struct thread thread;

/* This is the ready/delete queue of threads*/
struct stqueue
{
	thread *thread_in_queue;
	struct stqueue *next;
}; typedef struct stqueue stqueue;

/* Global variable for thread queues*/
stqueue *ready_queue;
stqueue *delete_queue;
thread *running_thread;
int tid_table[THREAD_MAX_THREADS];

/* thread starts by calling thread_stub. The arguments to thread_stub are the
 * thread_main() function, and one argument to the thread_main() function. */
void
thread_stub(void (*thread_main)(void *), void *arg)
{	

	interrupts_on();
	Tid ret;
	assert(interrupts_enabled());	
	thread_main(arg); // call thread_main() function with arg
	ret = thread_exit(THREAD_SELF);
	// we should only get here if we are the last thread. 
	assert(ret == THREAD_NONE);
	// all threads are done, so process should exit
	free(ready_queue);
	free(delete_queue);
	exit(0);
}

/*thread_init is similary to linked_list_init*/
void thread_init(void)
{	
	//Initialize ready queue 
	ready_queue = (stqueue*)malloc(sizeof(stqueue));
	assert(ready_queue);
	ready_queue->thread_in_queue = NULL;
	ready_queue->next = NULL;

	//Initialize delete queue f
	delete_queue = (stqueue*)malloc(sizeof(stqueue));
	delete_queue->thread_in_queue = NULL;
	delete_queue->next = NULL;

	// allocate the thread that is currently running
	running_thread = (thread *)malloc(sizeof(thread));
	assert(running_thread);
	
	
	//elements of the thread_initial
	running_thread->thread_id = 0;
	running_thread->thread_ucontext=NULL;

	// tid table
	int count;
	for (count=0; count<THREAD_MAX_THREADS; count++)
		tid_table[count] = 0;
	tid_table[0]=1;

}

Tid thread_create(void (*fn) (void *), void *parg)
{
	int enabled = interrupts_off();	
	assert(!interrupts_enabled());

	thread *thread_new = (thread *)malloc(sizeof(thread));
	thread_new->thread_ucontext = (struct ucontext*)malloc(sizeof(struct ucontext));
	int err = getcontext(thread_new->thread_ucontext);
	assert(!err);
	

	void* temp= (void *)malloc (THREAD_MIN_STACK);	

	if(temp==NULL)
	{		
		interrupts_set(enabled);
		return THREAD_NOMEMORY;
	}


	thread_new->thread_ucontext->uc_stack.ss_flags = 0;
	thread_new->thread_ucontext->uc_stack.ss_size = THREAD_MIN_STACK;

	
	thread_new->thread_ucontext->uc_mcontext.gregs[REG_RSP] = (unsigned long)temp+THREAD_MIN_STACK;
	thread_new->thread_ucontext->uc_mcontext.gregs[REG_RSP] = (unsigned long)thread_new->thread_ucontext->uc_mcontext.gregs[REG_RSP] -(thread_new->thread_ucontext->uc_mcontext.gregs[REG_RSP])%16 -8;
	thread_new->thread_ucontext->uc_mcontext.gregs[REG_RIP] = (unsigned long)&thread_stub;

	thread_new->thread_ucontext->uc_mcontext.gregs[REG_RDI] =  (unsigned long)fn;
	thread_new->thread_ucontext->uc_mcontext.gregs[REG_RSI] = (unsigned long)parg;

	// find next tid
	int count;
	bool found = false;
	int tid;
	for(count=0; count<THREAD_MAX_THREADS && !found; count++)
	{
		if(tid_table[count]==0)
		{
			tid = count;
			found = true;
		}
	}
	if(!found)
	{
		interrupts_set(enabled);	
		return THREAD_NOMORE;
	}		

	thread_new->thread_id = tid;
	tid_table[tid] = 1;

	stqueue *current_node = ready_queue->next;
	stqueue *prev_node = ready_queue;
	while (current_node != NULL)
	{
		prev_node = current_node;
		current_node = current_node->next;
	}

	stqueue *new_node = (stqueue*)malloc(sizeof(stqueue));
	prev_node->next = new_node;
	new_node->next = NULL;
	new_node->thread_in_queue = thread_new;
	interrupts_set(enabled);
	return new_node->thread_in_queue ->thread_id;
}

Tid thread_yield(Tid want_tid)
{
	int enabled = interrupts_off();	
	assert(!interrupts_enabled());

	// Check if delete queue is emtpy
	/*if (delete_queue->next != NULL)
	{
		stqueue *delete_node;
		while(delete_queue->next != NULL)		
		{
			delete_node = delete_queue->next;
			free(delete_node->thread_in_queue->thread_ucontext);
			free(delete_node->thread_in_queue);
			// Memory Leak! stack not free  !!!
			delete_queue->next = delete_node->next;
			free(delete_node);
		}
	}*/
	// Find running thread in ready queue
	

	if (running_thread == NULL)
	{
		interrupts_set(enabled);	
		return THREAD_FAILED; 
	}

	if(running_thread->thread_ucontext == NULL)
	{
		running_thread->thread_ucontext = (struct ucontext*)malloc(sizeof(struct ucontext)); 
		assert(running_thread->thread_ucontext);
	}

	//yield itself
	if (want_tid == THREAD_SELF || want_tid == running_thread->thread_id)
	{
	
		int setcontext_called = 0;		
		int err = getcontext(running_thread->thread_ucontext);
		assert(!err);
		if (setcontext_called == 1)
		{
			interrupts_set(enabled);	
			return (running_thread->thread_id);
		}		
		setcontext_called = 1;
		err = setcontext(running_thread->thread_ucontext);
		assert(!err);	
	}


	// yield any
	else if (want_tid == THREAD_ANY)
	{
	

		//find next running thread
		if (ready_queue->next == NULL)
		{
			interrupts_set(enabled);
			return THREAD_NONE;
		}
		stqueue *delete_node = ready_queue->next;
		ready_queue->next = delete_node->next;

		thread *end_running = running_thread;
		thread *start_running = delete_node->thread_in_queue;
		
		free(delete_node);
		stqueue *current_node = ready_queue->next;
		stqueue *prev_node = ready_queue;
		while (current_node != NULL)
		{
			prev_node = current_node;
			current_node = current_node->next;
		}
		stqueue *new_node = (stqueue*)malloc(sizeof(stqueue));
		prev_node->next = new_node;
		new_node->next = NULL;
		new_node->thread_in_queue = end_running;
		running_thread = start_running;
		int setcontext_called = 0;		
		int err = getcontext(end_running->thread_ucontext);
		assert(!err);

		if (setcontext_called == 1)
		{
			interrupts_set(enabled);
			return (start_running->thread_id);
		}						
		setcontext_called = 1;
		err = setcontext(start_running->thread_ucontext);
		assert(!err);
	}

	else
	{

		if(want_tid>THREAD_MAX_THREADS || want_tid <-2)
		{
			interrupts_set(enabled);
			return THREAD_INVALID;
		}
		if(want_tid< THREAD_MAX_THREADS && want_tid>=0 && tid_table[want_tid]==0)
		{
			interrupts_set(enabled);
			return THREAD_INVALID;
		}

		if (ready_queue->next == NULL)
		{
			interrupts_set(enabled);
			return THREAD_INVALID;
		}

		bool found = false;
		stqueue *current_node = ready_queue->next;
		stqueue *prev_node = ready_queue;

		while(current_node!=NULL && !found)
		{
			if(current_node->thread_in_queue->thread_id == want_tid)
			{
				prev_node->next = current_node->next;
				found = true;
			}
			else
			{
				prev_node = current_node;
               			current_node = current_node->next;

			}
		}
		if(!found)
		{
			interrupts_set(enabled);
			return THREAD_INVALID;
		}

		stqueue *delete_node = current_node;

		thread *end_running = running_thread;
		thread *start_running = delete_node->thread_in_queue;
		
		free(delete_node);

		current_node = ready_queue->next;
		prev_node = ready_queue;
		while (current_node != NULL)
		{
			prev_node = current_node;
			current_node = current_node->next;
		}
		stqueue *new_node = (stqueue*)malloc(sizeof(stqueue));
		prev_node->next = new_node;
		new_node->next = NULL;
		new_node->thread_in_queue = end_running;
		running_thread = start_running;

		int setcontext_called = 0;		
		int err = getcontext(end_running->thread_ucontext);
		assert(!err);

		if (setcontext_called == 1)
		{
			interrupts_set(enabled);
			return (start_running->thread_id);
		}						
		setcontext_called = 1;
		err = setcontext(start_running->thread_ucontext);
		assert(!err);
	}
	interrupts_set(enabled);	
	return THREAD_FAILED;
}

Tid thread_exit(Tid tid)
{	

	int enabled = interrupts_off();
	assert(!interrupts_enabled());
	if (running_thread == NULL)
	{
		interrupts_set(enabled);		
		return THREAD_FAILED; 
	}

	// delete running thread
	if(tid == running_thread->thread_id || tid == THREAD_SELF)
	{
		if (ready_queue->next == NULL)
		{
			interrupts_set(enabled);
			return THREAD_NONE;
		}
		stqueue *delete_node = ready_queue->next;
		ready_queue->next = delete_node->next;

		thread *end_running = running_thread;
		thread *start_running = delete_node->thread_in_queue;
		
		free(delete_node);
		stqueue *current_node = delete_queue->next;
		stqueue *prev_node = delete_queue;
		while (current_node != NULL)
		{
			prev_node = current_node;
			current_node = current_node->next;
		}
		stqueue *new_node = (stqueue*)malloc(sizeof(stqueue));
		prev_node->next = new_node;
		new_node->next = NULL;
		new_node->thread_in_queue = end_running;
		running_thread = start_running;
		tid_table[end_running->thread_id] = 0;
		int err = setcontext(start_running->thread_ucontext);
		assert(!err);	
	}
	else if (tid == THREAD_ANY)
	{
		if (ready_queue->next  == NULL)
		{
			interrupts_set(enabled);
			return THREAD_NONE;
		}

		stqueue *delete_node = ready_queue->next;
		ready_queue->next = delete_node->next;

		thread *end_thread = delete_node->thread_in_queue;
		
		free(delete_node);

		stqueue *current_node = delete_queue->next;
		stqueue *prev_node = delete_queue;
		while (current_node != NULL)
		{
			prev_node = current_node;
			current_node = current_node->next;
		}
		stqueue *new_node = (stqueue*)malloc(sizeof(stqueue));
		prev_node->next = new_node;
		new_node->next = NULL;
		new_node->thread_in_queue = end_thread;
		tid_table[end_thread->thread_id] = 0;
		interrupts_set(enabled);
		return end_thread->thread_id;
	}

	else
	{
		if(tid> THREAD_MAX_THREADS || tid <-2)
		{
			interrupts_set(enabled);
			return THREAD_INVALID;
		}
		if(tid< THREAD_MAX_THREADS && tid>=0 && tid_table[tid]==0)
		{
			interrupts_set(enabled);
			return THREAD_INVALID;
		}

		if (ready_queue->next == NULL)
		{
			interrupts_set(enabled);
			return THREAD_INVALID;
		}

		bool found = false;
		stqueue *current_node = ready_queue->next;
		stqueue *prev_node = ready_queue;

		while(current_node!=NULL && !found)
		{
			if(current_node->thread_in_queue->thread_id == tid)
			{
				prev_node->next = current_node->next;
				found = true;
			}
			else
			{
				prev_node = current_node;
                current_node = current_node->next;

			}
		}
		if(!found)
		{
			interrupts_set(enabled);
			return THREAD_INVALID;
		}

		stqueue *delete_node = current_node;
		thread *end_thread = delete_node->thread_in_queue;
		free(delete_node);

		current_node = delete_queue->next;
		prev_node = delete_queue;
		while (current_node != NULL)
		{
			prev_node = current_node;
			current_node = current_node->next;
		}
		stqueue *new_node = (stqueue*)malloc(sizeof(stqueue));
		prev_node->next = new_node;
		new_node->next = NULL;
		new_node->thread_in_queue = end_thread;
		tid_table[end_thread->thread_id]=0;
		
	interrupts_set(enabled);	
	return new_node->thread_in_queue->thread_id;
}

		
	interrupts_set(enabled);
	return THREAD_FAILED;
}

/*******************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 *******************************************************************/

/* This is the wait queue structure */
struct wait_queue {
	thread *thread_wait;
	struct wait_queue *next;
};typedef struct wait_queue wait_queue;

struct wait_queue *wait_queue_create()
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());
	struct wait_queue *wq;
	wq = malloc(sizeof(struct wait_queue));
	assert(wq);
	wq->thread_wait = NULL;
	wq->next = NULL;
	interrupts_set(enabled);
	return wq;
}

void wait_queue_destroy(struct wait_queue *wq)
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());
	if (wq->next != NULL)
		thread_wakeup(wq, 1);
	free(wq);
	interrupts_set(enabled);
}

Tid thread_sleep(struct wait_queue *queue)
{

	int enabled = interrupts_off();
	assert(!interrupts_enabled());
	if(queue == NULL)
	{
		interrupts_set(enabled);
		return THREAD_INVALID;
	}

	if (ready_queue->next == NULL)
	{
		interrupts_set(enabled);
		return THREAD_NONE;
	}
	
	stqueue *delete_node = ready_queue->next;
	ready_queue->next = delete_node->next;

	thread *sleep_thread = running_thread;
	thread *start_running = delete_node->thread_in_queue;
		
	free(delete_node);
	
	wait_queue *current_node = queue->next;
	wait_queue *prev_node = queue;
	while (current_node != NULL)
	{
		prev_node = current_node;
		current_node = current_node->next;
	}
	wait_queue *new_node = (wait_queue*)malloc(sizeof(wait_queue));
	prev_node->next = new_node;
	new_node->next = NULL;
	new_node->thread_wait = sleep_thread;
	running_thread = start_running;

	int setcontext_called = 0;		
	int err = getcontext(sleep_thread->thread_ucontext);
	assert(!err);

	if (setcontext_called == 1)
	{
		interrupts_set(enabled);
		return (start_running->thread_id);
	}						
	setcontext_called = 1;
	err = setcontext(start_running->thread_ucontext);
	assert(!err);

	interrupts_set(enabled);
	return THREAD_FAILED;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int thread_wakeup(struct wait_queue *queue, int all)
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());
	if(queue == NULL || queue->next == NULL)
	{
		interrupts_set(enabled);
		return 0;
	}

	if(all == 1) //wake up all
	{
		
		int count = 0;
		while (queue->next != NULL)
		{
			wait_queue * delete_node = queue->next;
			queue->next = delete_node->next;
        	thread *get_ready = delete_node->thread_wait;
        	free(delete_node);

			stqueue *current_node = ready_queue->next;
			stqueue *prev_node = ready_queue;
			while (current_node != NULL)
			{
				prev_node = current_node;
				current_node = current_node->next;
			}
			stqueue *new_node = (stqueue*)malloc(sizeof(stqueue));
			prev_node->next = new_node;
			new_node->next = NULL;
			new_node->thread_in_queue = get_ready;
			count = count +1;
		}
		interrupts_set(enabled);
		return count;
	}	


	else
	{
        wait_queue * delete_node = queue->next;
        queue->next = delete_node->next;
        thread *get_ready = delete_node->thread_wait;
        free(delete_node);

		stqueue *current_node = ready_queue->next;
		stqueue *prev_node = ready_queue;
		while (current_node != NULL)
		{
			prev_node = current_node;
			current_node = current_node->next;
		}
		stqueue *new_node = (stqueue*)malloc(sizeof(stqueue));
		prev_node->next = new_node;
		new_node->next = NULL;
		new_node->thread_in_queue = get_ready;
		interrupts_set(enabled);
		return 1;
	}

	interrupts_set(enabled);     
	return 0;
}

struct lock {
	bool acquired;
	thread *lock_thread;
	wait_queue *lock_wait;
};

struct lock *
lock_create()
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());

	struct lock *lock;
	lock = malloc(sizeof(struct lock));
	assert(lock);
	lock->acquired = false;
	lock->lock_thread = NULL;
	lock->lock_wait = wait_queue_create();
	interrupts_set(enabled);
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());

	assert(lock != NULL);
	while(1)
	{
		if (lock->acquired == false)
		{
			wait_queue_destroy(lock->lock_wait);
			free(lock);
			interrupts_set(enabled);
			return;
		}
		else
			thread_sleep(lock->lock_wait);
	}
}

void
lock_acquire(struct lock *lock)
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());
	assert(lock != NULL);
	while(1)
	{
		if(!lock->acquired)
		{
			lock->acquired = true;
			lock->lock_thread = running_thread;
			interrupts_set(enabled);
			return;
		}
		else
			thread_sleep(lock->lock_wait);
	}

}

void
lock_release(struct lock *lock)
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());
	
	assert(lock != NULL);
	
	lock->acquired = false;
	lock->lock_thread = NULL;
	thread_wakeup(lock->lock_wait, 1);
	
	interrupts_set(enabled);
	return;
}

struct cv {
	wait_queue *cv_wait;
};

struct cv *
cv_create()
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());
	
	struct cv *cv;
	cv = malloc(sizeof(struct cv));
	assert(cv);

	cv->cv_wait = wait_queue_create();

	interrupts_set(enabled);
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());

	assert(cv != NULL);

	wait_queue_destroy(cv->cv_wait);

	free(cv);
	interrupts_set(enabled);
	return;
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());

	assert(cv != NULL);
	assert(lock != NULL);
	
	assert(lock->lock_thread == running_thread);
	lock_release(lock);
	thread_sleep(cv->cv_wait);
	lock_acquire(lock);
	interrupts_set(enabled);
	return;
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());

	assert(cv != NULL);
	assert(lock != NULL);

	assert(lock->lock_thread == running_thread);
	thread_wakeup(cv->cv_wait, 0);
	interrupts_set(enabled);
	return;
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	int enabled = interrupts_off();
	assert(!interrupts_enabled());

	assert(lock->lock_thread == running_thread);
	assert(cv != NULL);
	assert(lock != NULL);
	thread_wakeup(cv->cv_wait, 1);
	interrupts_set(enabled);
	return;
}
