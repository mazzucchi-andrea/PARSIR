#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <run.h>
#include <pthread.h>
#include <stdalign.h>
#include <speculation.h>

#include <queue.h>
#include <random.h>

#define STACKABLE_OBJECTS (50)

object_status speculation[OBJECTS];//an entry of this array needs to be managed in separation 
				   //via the corresponding speculation_lock[] acquisition
alignas(64) lock_buffer speculation_locks[OBJECTS];

extern double filter_message[OBJECTS];
extern log_element log_queue[OBJECTS];


__thread int object_stack[STACKABLE_OBJECTS];
__thread int stack_index = -1;

int speculation_init(void) {
    int i;
    int j;

    for (j = 0; j < OBJECTS; j++) {
            pthread_spin_init(&(speculation_locks[j].lock), PTHREAD_PROCESS_PRIVATE);
	    filter_message[j] = 0.0 - epsilon;
            log_queue[j].the_element = NULL;
            log_queue[j].send_time = 0.0 - epsilon;
            log_queue[j].next = NULL;
            log_queue[j].prev = NULL;
            log_queue[j].first = NULL;
            log_queue[j].last = NULL;
    }
    return 1;
}


void object_lock(int object){
	pthread_spin_lock(&speculation_locks[object].lock);
	return;
}

void object_unlock(int object){
	pthread_spin_unlock(&speculation_locks[object].lock);
	return;
}

void put_into_stack(int object){
	stack_index++;
	if(stack_index >= STACKABLE_OBJECTS){//no more room in the stack
		printf("no more room into the object stack\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	object_stack[stack_index] = object;
}

void put_head_into_stack(int object){
	int i;
	stack_index++;
	if(stack_index >= STACKABLE_OBJECTS){//no more room in the stack
		printf("no more room into the object stack\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	for(i = stack_index; i > 0; i--) object_stack[i] = object_stack[i-1];//make room for a lower priority object 
									     //to be put in the 0-th entry
	object_stack[0] = object;//finalize the insertion
}

int get_from_stack(int * object){
	if (stack_index == -1) return 0;
	*object = object_stack[stack_index];
	stack_index--;
	return 1;
}

#ifdef NADA
void rollback_speculation_queue(int object, double rollback_time){
    fallback_slot *q = &speculation_queue[object];
    queue_elem *cur;
    queue_elem *prev;

    if (q->head == NULL)
        return;

    cur = q->tail;

    /* walk backwards removing elements >= rollback_time */
    while (cur && cur->send_time >= rollback_time) {
        prev = cur->prev;
        free(cur);
        cur = prev;
        __sync_fetch_and(&speculation_events, -1);
    }

    if (cur == NULL) {
        /* queue fully rolled back */
        q->head = NULL;
        q->tail = NULL;
    } else {
        /* cut the queue */
        cur->next = NULL;
        q->tail = cur;
    }
}

void  rollback_retractable_queue(int object, double rollback_time){


}
#endif


void restore_state(int object){

}


int run_rollback(int object, double rollback_time){
	rollback_speculation_queue(object,rollback_time);
	log_rollback(object,rollback_time);
	restore_state(object);//here we default to the initial state of the current epoch
			      //but remenber to reset the object_status structure
	restore_retractable_events(object);//we somply reput stuff in the input queue
	return 0;
}
