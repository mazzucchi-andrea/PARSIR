#include <pthread.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "queue.h"
#include "random.h"
#include "run.h"
#include "speculation.h"

#if GRID_CKPT
#include "grid_ckpt.h"
#elif CHUNK_BASED
#include "chunk_ckpt.h"
#endif

#define STACKABLE_OBJECTS (OBJECTS)

object_status speculation[OBJECTS]; // an entry of this array needs to be managed in separation
                                    // via the corresponding speculation_lock[] acquisition
alignas(64) lock_buffer speculation_locks[OBJECTS];

extern double filter_message[OBJECTS];
extern log_element log_queue[OBJECTS];

__thread int object_stack[STACKABLE_OBJECTS];
__thread int stack_index = -1;

int get_stack_index(){
    return stack_index;
}

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

void object_lock(int object) {
    pthread_spin_lock(&speculation_locks[object].lock);
    return;
}

void object_unlock(int object) {
    pthread_spin_unlock(&speculation_locks[object].lock);
    return;
}

void put_into_stack(int object) {
    stack_index++;
    printf("putting %d into the stack\n", object);
    fflush(stdout);
    if (stack_index >= STACKABLE_OBJECTS) { // no more room in the stack
        printf("no more room into the object stack\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    object_stack[stack_index] = object;
}

void put_head_into_stack(int object) {
    int i;
    stack_index++;
    printf("putting %d head into the stack\n", object);
    fflush(stdout);
    if (stack_index >= STACKABLE_OBJECTS) { // no more room in the stack
        printf("no more room into the object stack\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    for (i = stack_index; i > 0; i--) {
        object_stack[i] = object_stack[i - 1]; // make room for a lower priority object
                                               // to be put in the 0-th entry
    }
    object_stack[0] = object; // finalize the insertion
}

int get_from_stack(int *object) {
    if (stack_index == -1) {
        return 0;
    } 
    printf("getting %d from the stack\n", object_stack[stack_index]);
    fflush(stdout);
    *object = object_stack[stack_index];
    stack_index--;
    return 1;
}

void restore_state(int object) {
    AUDIT {
        printf("object %d - restore_state called\n", object);
        fflush(stdout);
    }
    restore_object(object);
    // rollback grid/chunk
}

int run_rollback(int object, double rollback_time) {
    {
        printf("object %d - rollback with rollback_time %e\n", object, rollback_time);
        fflush(stdout);
    }
    rollback_speculation_queue(object, rollback_time);
    log_rollback(object, rollback_time);
    restore_state(object);              // here we default to the initial state of the current epoch
                                        // but remember to reset the object_status structure
    printf("Queue status before restore_retractable_events\n");
    print_queues_status(object);
    restore_retractable_events(object); // we simply reput stuff in the input queue
    return 0;
}
