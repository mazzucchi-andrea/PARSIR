#include <pthread.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "engine.h"
#include "queue.h"
#include "random.h"
#include "setup.h"

#ifdef SPECULATION
#include "speculation.h"
#endif
#if GRID_CKPT
#include "grid_ckpt.h"
#elif CHUNK_BASED
#include "chunk_ckpt.h"
#endif

void speculation_queue_flush(void);
void retractable_queue_flush(void);

slot queue[OBJECTS][NUM_SLOTS];
alignas(64) lock_buffer locks[OBJECTS][NUM_SLOTS];
// lock_buffer locks[OBJECTS][NUM_SLOTS];

#ifdef SPECULATION
slot retractable_queue[OBJECTS];
log_element log_queue[OBJECTS]; // this queue will just record <send_time,buffer_address> entries
#endif

double volatile current_min_limit = 0.0;
double volatile current_max_limit = 0.0 + LOOKAHEAD;
int volatile current_index = 0;

long pending_events __attribute__((aligned(64))) = 0;
long object_identifiers __attribute__((aligned(64))) = 0;
#ifdef SPECULATION
long shadow_object_identifiers __attribute__((aligned(64))) = 0;
long retractable_object_identifiers __attribute__((aligned(64))) = 0;
#endif
long object_identifiers_vector[MAX_NUMA_NODES] __attribute__((aligned(64))) = {[0 ... MAX_NUMA_NODES - 1] = 0};
int end = 0;

#ifdef SPECULATION
long speculation_events __attribute__((aligned(64))) = 0;
long retractable_events __attribute__((aligned(64))) = 0;
double filter_message[OBJECTS] = {(0.0 - epsilon)}; // this just initializes the 0-th entry to show
                                                    // the value that should be written across all
extern object_status speculation[OBJECTS];
#endif

__thread int seen_empty_slot = 0;
__thread int my_index = 0;
__thread fallback_slot fallback_queue; // WE INITIALIZE VIA EMPTY ZERO MEMORY = { .head = NULL , .tail = NULL };

#ifdef SPECULATION
fallback_slot speculation_queue[OBJECTS] = {
    0}; // WE INITIALIZE VIA EMPTY ZERO MEMORY = { .head = NULL , .tail = NULL };
#endif

__thread unsigned me;
__thread unsigned target = -1;

// we use '_' here just to discriminate from the
// corresponding  non TLS global variables
__thread int *_c;
__thread int *_min;
__thread int *_max;
// these are used for NUMA aware workload distribution
__thread int myNUMAnode;
__thread int myNUMAindex;
__thread int stealNUMAindex;
__thread int TOT_NUMA_NODES;

void whoami(unsigned my_id) {
    AUDIT printf("just audit whoami: %u\n", my_id);
    me = my_id;
    _c = getcounter();
    _min = getmin();
    _max = getmax();
    myNUMAindex = myNUMAnode = get_NUMAnode();
    stealNUMAindex = 0;
    TOT_NUMA_NODES = get_totNUMAnodes();
}

void verify_queues(void) {
    for (int i = 0; i < OBJECTS; i++) {
        if (queue[i][my_index].head.next != &queue[i][my_index].tail) {
            printf("object %d - queue not empty first object timestamp is %e\n", i,
                   queue[i][my_index].head.next->timestamp);
            printf("object %d - in stack: %d - state: %d - standing_rollback: %d\n", i, speculation[i].in_stack,
                   speculation[i].the_state, speculation[i].standing_rollback);
            //exit(EXIT_FAILURE);
            print_queues_status(i);
        }
    }
}

void verify_queue(int object) {
    if (queue[object][my_index].head.next != &queue[object][my_index].tail) {
        printf("object %d - queue not empty first object timestamp is %e\n", object,
               queue[object][my_index].head.next->timestamp);
        exit(EXIT_FAILURE);
    }
}

void queue_init(void) {
    int i;
    int j;
    queue_elem *head;
    queue_elem *tail;

    for (j = 0; j < OBJECTS; j++) {
        head = &retractable_queue[j].head;
        tail = &retractable_queue[j].tail;
        head->next = tail; // setup initial double linked list
        head->prev = NULL; 
        tail->prev = head;
        tail->next = NULL;
        head->timestamp = -1; // setup initial timestamp value
        tail->timestamp = -1;
        for (i = 0; i < NUM_SLOTS; i++) {
            head = &queue[j][i].head;
            tail = &queue[j][i].tail;
            head->next = tail; // setup initial double linked list
            tail->prev = head;
            head->timestamp = -1; // setup initial timestamp value
            tail->timestamp = -1;
            if (pthread_spin_init(&locks[j][i].lock, PTHREAD_PROCESS_PRIVATE)) {
                printf("Object %d - Failing pthread_spin_init for slot %d\n", j, i);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void update_timing(void) {
    current_min_limit += LOOKAHEAD;
    current_max_limit += LOOKAHEAD;
    current_index = (current_index + 1) % NUM_SLOTS;
    int i;

    AUDIT {
        printf("updating timing - current min is %e - current max is %e - current index is %d\n", current_min_limit,
               current_max_limit, current_index);
        fflush(stdout);
    }

    object_identifiers = 0;

#ifdef SPECULATION
    shadow_object_identifiers = 0;
    retractable_object_identifiers = 0;
#endif

    for (i = 0; i < MAX_NUMA_NODES; i++) {
        object_identifiers_vector[i] = 0;
    }

#ifdef SPECULATION
    if (!pending_events && !speculation_events) {
#else
    if (!pending_events) {
#endif
        end = 1;
    }
}

int queue_insert(queue_elem *elem) {
    queue_elem *current;
    queue_elem *tail;
    int index;
    int dest;

    AUDIT printf("just audit who I am: %u\n", me);

    if (elem->timestamp < current_min_limit) {
        printf("object %d - illegal queue insert - timestamp is %e - min limit is %e\n", elem->destination,
               elem->timestamp, current_min_limit);
        return -1;
    }

    if (elem->timestamp >= current_max_limit) {
        // here we make a tail insert - there will be no next
	printf("inserting avent with timestamp %e in the fallback queue\n",elem->timestamp);
	fflush(stdout);
        elem->next = NULL;
        if (fallback_queue.head == NULL) {
            elem->prev = NULL;
            fallback_queue.head = elem;
            fallback_queue.tail = elem;
        } else {
            elem->prev = fallback_queue.tail;
            fallback_queue.tail->next = elem;
            fallback_queue.tail = elem;
        }
        __sync_fetch_and_add(&pending_events, 1);
        return 0;
    }

    index = (int)((elem->timestamp) / (double)SLOT_LEN);
    index = index % NUM_SLOTS;

    AUDIT {
        printf("object %d - inserting event with timestamp %e in slot %d\n", elem->destination, elem->timestamp, index);
        fflush(stdout);
    }

    dest = elem->destination;

    current = &queue[dest][index].head;
    tail = &queue[dest][index].tail;

    pthread_spin_lock(&locks[dest][index].lock);

    while (current->timestamp <= elem->timestamp && current->next != tail) {
        current = current->next;
    }
    // printf("queue_insert current timestamp %e elem timestamp %e\n", current->timestamp, elem->timestamp);
    if (current->timestamp <= elem->timestamp) {
        elem->next = current->next; // link to the subsequent element
        current->next = elem;

        elem->next->prev = elem; // relink the previoous elements
        elem->prev = current;
    } else {
        elem->next = current; // link to the subsequent element
        elem->prev = current->prev;
        current->prev->next = elem;
        current->prev = elem;
    }

    __sync_fetch_and_add(&pending_events, 1); // there is one more element in the queue

    pthread_spin_unlock(&locks[dest][index].lock);

    return 0;
}

void print_queues_status(int object) {
    queue_elem *curr = queue[object][my_index].head.next;
    printf("printing the queue of object %d\n",object);
    fflush(stdout);
    while (curr != &queue[object][my_index].tail) {
        printf("object %d - event in queue with timestamp %e\n", object, curr->timestamp);
        fflush(stdout);
        curr = curr->next;
    }
    curr = retractable_queue[object].head.next;
    printf("printing the retractble queue of object %d\n",object);
    fflush(stdout);
    while (curr != &retractable_queue[object].tail) {
        printf("object %d - event in retractable_queue with timestamp %e\n", object, curr->timestamp);
        fflush(stdout);
        curr = curr->next;
    }
}

void fallback_check(void) {
    queue_elem *temp = fallback_queue.head; // the fallback_queue is __thread hence
                                            // we already run isolated on this queue
    queue_elem *aux;
    queue_elem *current;
    queue_elem *tail;
    int index;
    int dest;

    while (temp) {
        if (temp->timestamp >= current_max_limit) {
            temp = temp->next;
        }

        else {
            aux = temp->next;
            if (fallback_queue.head == temp) {
                fallback_queue.head = temp->next;
            }
            if (fallback_queue.tail == temp) {
                fallback_queue.tail = temp->prev;
            }
            if (temp->next) {
                temp->next->prev = temp->prev;
            }
            if (temp->prev) {
                temp->prev->next = temp->next;
            }

            index = (int)((temp->timestamp) / (double)SLOT_LEN);
            index = index % NUM_SLOTS;
            dest = temp->destination;

 //           current = queue[dest][index].head.next; // here current is not the object but
            current = &queue[dest][index].head; // here current is not the object but
                                                    // the target queue head
            tail = &queue[dest][index].tail;

            {
                printf("object %d - queue_insert (from fallback) for an element with timestamp %e\n", temp->destination,
                       temp->timestamp);
                fflush(stdout);
            }

            pthread_spin_lock(&locks[dest][index].lock);

            while (current->timestamp <= temp->timestamp && current->next != tail) {
                current = current->next;
            }
	    if (current->timestamp <= temp->timestamp) {
       		 temp->next = current->next; // link to the subsequent element
       		 current->next = temp;
	
       		 temp->next->prev = temp; // relink the previoous elements
       		 temp->prev = current;
    		} else {
       		 temp->next = current; // link to the subsequent element
       		 temp->prev = current->prev;
       		 current->prev->next = temp;
       		 current->prev = temp;
    	}

//	    printf("after while\n");
//	    fflush(stdout);
 //           temp->next = current->next; // link to the subsequent element
//	    printf("after 1st\n");
//	    fflush(stdout);
 //           current->next = temp;
//	    printf("after 2nd\n");
//	    fflush(stdout);
 //           temp->next->prev = temp; // relink the previous elements
//	    printf("after 3rd\n");
//	    fflush(stdout);
 //           temp->prev = current;
//	    printf("after 4rt\n");
//	    fflush(stdout);

            pthread_spin_unlock(&locks[dest][index].lock);

            temp = aux;
        }
    }
}

queue_elem *queue_extract() {
    int index;
    queue_elem *head;
    queue_elem *tail;
    queue_elem *elem;
    int to_rollback;
#ifdef SPECULATION
    double rollback_time;
   // if (target == -1 || target > OBJECTS) {
   if (get_stack_index() >= 0) {
        get_from_stack(&target);
        if (target != -1) {
            speculation[target].in_stack = 0;
        }
    }
#endif

    AUDIT printf("thread %d - extraction with target %d\n", me, target);

start:
#ifndef NUMA_BALANCING
    if (target == -1) {
        target = __sync_fetch_and_add(&object_identifiers, 1);
    }
#else
    if (target == -1) {
    retry:
        target = __sync_fetch_and_add(&object_identifiers_vector[myNUMAindex], 1);
        if (target >= _c[myNUMAindex]) {
            if (stealNUMAindex < TOT_NUMA_NODES) {
                stealNUMAindex++;
                myNUMAindex = (myNUMAindex + 1) % TOT_NUMA_NODES;
                goto retry;
            } else {
                target = OBJECTS;
            }
        } else {
            target += _min[myNUMAindex];
        }
    }

#endif

redo:
    if (end) {
        return NULL;
    }
    if (target < OBJECTS) {
#ifdef SPECULATION
        object_lock(target);
        speculation[target].already_taken = 1;
        rollback_time = 0.0;
        if (speculation[target].standing_rollback) {
            rollback_time = speculation[target].causality_violation_time;
            speculation[target].standing_rollback = 0;
            speculation[target].current_time = current_min_limit;
            filter_message[target] = rollback_time - epsilon;
        }
        object_unlock(target);
        if (rollback_time > 0.0) {
            run_rollback(target, rollback_time); // we restore the current epoch initial state of the object
                                                 // after we need to run this object as a normal execution
                                                 // but we need to avoid new events production up to the rollback_time
                                                 // that has been flushed to the filter_message[] entry of the object
            printf("Queues status after rollback\n");
            print_queues_status(target);
        }
#endif
        object_lock(target);
        index = my_index;
        head = &queue[target][index].head;
        tail = &queue[target][index].tail;

        if (head->next == tail) { // the current slot is empty - try with another target
            speculation[target].the_state = FREE;
            object_unlock(target);
#ifndef NUMA_BALANCING
            target = __sync_fetch_and_add(&object_identifiers, 1);
#else
            target = __sync_fetch_and_add(&object_identifiers_vector[myNUMAindex], 1);
            if (target >= _c[myNUMAindex]) {
                goto retry;
            } else {
                target += _min[myNUMAindex];
            }
#endif
            goto redo;
        }
        object_unlock(target);
    } else {
        AUDIT {
            printf("found empty slot with index %d\n", index);
            fflush(stdout);
        }
#ifdef SPECULATION
        barrier();
        verify_queues();
        speculation_queue_flush();
        retractable_queue_flush();
#endif
        if (barrier()) {
            update_timing(); // this call updates the queue layout and releases the objects taken by threads in the last
                             // epoch
        }
        barrier();

        my_index = current_index;
        target = -1;
        // reset stuff for NUMA aware workload distribution
        myNUMAindex = myNUMAnode;
        stealNUMAindex = 0;
        fallback_check();
        goto start;
    }

#ifdef SPECULATION
    object_lock(target);
#endif
    pthread_spin_lock(&locks[target][index].lock);

    if (head->next == tail) { // the current slot is empty
        pthread_spin_unlock(&locks[target][index].lock);
#ifdef SPECULATION
        if (speculation[target].in_stack == 1) {
            printf("ERROR: stack management violation\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
/*         if (speculation[target].standing_rollback) {
            put_into_stack(target);
            speculation[target].the_state = BUSY;
            speculation[target].in_stack = 1;
        } else {
            speculation[target].the_state = FREE;
        } */
        speculation[target].the_state = FREE;
        object_unlock(target);
#endif
        if (end) {
            return NULL;
        } else {
            goto redo;
        }
    }

    // regular extraction from a non-empty slot

    elem = head->next;

    head->next = elem->next;
    elem->next->prev = head;

    elem->next = NULL;
    elem->prev = NULL;

    __sync_fetch_and_add(&pending_events, -1);

    pthread_spin_unlock(&locks[target][index].lock);

#ifdef SPECULATION
    speculation[target].current_time = elem->timestamp;
    object_unlock(target);
#endif
    return elem;
}

#ifdef SPECULATION

void log_the_send(queue_elem *the_elem, int current_object, double current_time) {
    log_element *queue = &log_queue[current_object];
    log_element *node;

    node = malloc(sizeof(log_element));
    if (!node) {
        return;
    }

    node->the_element = the_elem;
    node->send_time = current_time;
    node->next = NULL;
    node->prev = queue->last;
    node->first = NULL; /* unused in nodes */
    node->last = NULL;  /* unused in nodes */

    if (queue->last) {
        /* queue not empty */
        queue->last->next = node;
    } else {
        /* first element */
        queue->first = node;
    }

    queue->last = node;
}

void flush_log(int object) {
    log_element *queue = &log_queue[object];
    log_element *cur;
    log_element *next;

    cur = queue->first;

    while (cur != NULL) {
        next = cur->next;
        free(cur);
        cur = next;
    }

    queue->first = NULL;
    queue->last = NULL;
}

int speculation_queue_insert(queue_elem *elem) {
    queue_elem *current;
    queue_elem *tail;
    int index;
    int dest;
    int source;
    int destination;

    source = get_current();
    elem->send_time = get_current_time();

    AUDIT printf("just audit who I am: %u\n", me);

    if (elem->timestamp <
        (current_min_limit)) { // with speculaton we have a LOOKAHEAD wide time window for straggler acceptance
        printf("object %d - illegal speculation queue insert - timestamp is %e - min speculation limit is %e\n",
               current, elem->timestamp, current_min_limit - LOOKAHEAD);
        return -1;
    }

    if (elem->timestamp >= current_min_limit + LOOKAHEAD) { // this insertion into the speculation queue will be flushed
                                                            // to the actual input queue of the destination object if
                                                            // the source will not rollback the event generation
//        AUDIT {
            printf("inserting in speculation queue an event with timestamp %e for object %d\n", elem->timestamp,
                   elem->destination);
            fflush(stdout);
 //       }
        // here we make a tail insert - there will be no next
        elem->next = NULL;
        if (speculation_queue[source].head == NULL) {
            elem->prev = NULL;
            speculation_queue[source].head = elem;
            speculation_queue[source].tail = elem;
        } else {
            elem->prev = speculation_queue[source].tail;
            speculation_queue[source].tail->next = elem;
            speculation_queue[source].tail = elem;
        }
        __sync_fetch_and_add(&speculation_events, 1);
        return 0;
    }

    // insert into the current epoch - can give rise to rollback
    // in this case we need to log the address of elem into a log
    // so that we can use the address for retracting the elem insertion

    log_the_send(elem, source, get_current_time());

    destination = elem->destination;
    object_lock(destination);
    if (speculation[destination].current_time >= elem->timestamp) {
        printf("Rollback needed for object %d (current_time %e) with rollback_time %e\n", elem->destination,
               speculation[destination].current_time, elem->timestamp);
        if (speculation[destination].standing_rollback &&
            speculation[destination].causality_violation_time > elem->timestamp) {
            speculation[destination].causality_violation_time = elem->timestamp;
        }
        if (!speculation[destination].standing_rollback) {
            speculation[destination].standing_rollback = 1;
            speculation[destination].causality_violation_time = elem->timestamp;
        }
        queue_insert(elem);
        if (speculation[destination].standing_rollback &&
            (speculation[destination].the_state == FREE)) { // get the oject for processing
		printf("entered here\n");
		fflush(stdout);
            speculation[destination].the_state = BUSY;
            put_into_stack(destination);
            speculation[destination].in_stack = 1;
            if (!speculation[source].in_stack) {
                put_head_into_stack(source);
                speculation[source].in_stack = 1;
            }
            target = -1;
        }
    } else {
        queue_insert(elem);
        {
            printf("object %d - inserted event for object %d in the current epoch with timestamp %e\n", source, destination, elem->timestamp);
            fflush(stdout);
        }
        if ((speculation[destination].the_state == FREE) &&
            (speculation[destination].already_taken == 1)) { // get the object for processing
		printf("well I'm running here\n");
		fflush(stdout);
            speculation[destination].the_state = BUSY;
	    if( speculation[destination].in_stack == 0){
            	put_into_stack(destination);
            	speculation[destination].in_stack = 1;
            	if (speculation[source].in_stack == 0) {
              	  put_head_into_stack(source);
               	 speculation[source].in_stack = 1;
           	 }
	    }
            target = -1;
        }
    }
    object_unlock(destination);
    /* if (destination < object_identifiers_vector[myNUMAindex]) {
        __atomic_store_n(&object_identifiers_vector[myNUMAindex], destination, __ATOMIC_RELAXED);
    } */

    return 0;
}

void speculation_queue_flush(void) {
    queue_elem *temp; // the fallback_queue is __thread hence
                      // we already run isolated on this queue
    queue_elem *aux;
    queue_elem *current;
    queue_elem *tail;
    unsigned target_object;

    AUDIT {
        printf("flush of the speculation queue called\n");
        fflush(stdout);
    }

flush_another:
    target_object = __sync_fetch_and_add(&shadow_object_identifiers, 1);
    AUDIT printf("flushing for object %d\n", target_object);
    if (target_object < OBJECTS) {
        speculation[target_object].causality_violation_time = 0.0;
        speculation[target_object].standing_rollback = 0;
        speculation[target_object].the_state = FREE;
        speculation[target_object].already_taken = 0;
        filter_message[target_object] = current_min_limit + LOOKAHEAD - epsilon;

        flush_log(target_object);

        queue_elem *temp = speculation_queue[target_object].head;
        if (!temp) {
            AUDIT {
                printf("speculation queue of object %d is empty\n", target_object);
                fflush(stdout);
            }
        }

        while (temp) {
            aux = temp->next;
            if (speculation_queue[target_object].head == temp) {
                speculation_queue[target_object].head = temp->next;
            }
            if (speculation_queue[target_object].tail == temp) {
                speculation_queue[target_object].tail = temp->prev;
            }
            if (temp->next) {
                temp->next->prev = temp->prev;
            }
            if (temp->prev) {
                temp->prev->next = temp->next;
            }

            queue_insert(temp);
            AUDIT printf("object %d - inserting in the queue an event with timestamp %e for object %d\n", target_object,
                         temp->destination, temp->timestamp);
            temp = aux;
            __sync_fetch_and_add(&speculation_events, -1);
        }
    } else {
        return;
    }
    {
        printf("object %d - set checkpoint at %e\n", target_object, current_min_limit + LOOKAHEAD);
        fflush(stdout);
    }
    set_ckpt(target_object);
    goto flush_another;
}

void retractable_queue_flush(void) {
    unsigned target_object;

    AUDIT {
        printf("retractable_queue_flush called\n");
        fflush(stdout);
    }

flush_another:
    target_object = __sync_fetch_and_add(&retractable_object_identifiers, 1);
    if (target_object < OBJECTS) {
        queue_elem *curr = retractable_queue[target_object].head.next;
        queue_elem *tail = &retractable_queue[target_object].tail;
        queue_elem *next;
        while (curr != &retractable_queue[target_object].tail) {
            next = curr->next;
            free(container_of(curr, event, q));
            __sync_fetch_and_add(&retractable_events, -1);
            curr = next;
        }
        retractable_queue[target_object].head.next = tail;
        tail->prev = &retractable_queue[target_object].head;

    } else {
        return;
    }
    goto flush_another;
}

// this function is used to put into the per-object retractable queue an event that has been processed in the current
// epoch
void retractable_queue_insert(queue_elem *elem) {
    queue_elem *current;
    queue_elem *tail;
    int index;
    int dest;
    int source;

    dest = elem->destination;

    AUDIT printf("just audit who I am: %u\n", me);

    if (elem->timestamp <
        current_min_limit) { // with speculation we have a LOOKAHEAD wide time window for straggler acceptance
        printf("illegal retractable queue insert - object is %d - timestamp is %e - min speculation limit is %e\n",
               dest, elem->timestamp, current_min_limit);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    if (elem->timestamp >= (current_min_limit + LOOKAHEAD)) {
        printf(
            "illegal retractable queue insert - timestamp is %e - it oversteps the epoch limit plus lookahead (%e)\n",
            elem->timestamp, current_min_limit + LOOKAHEAD);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    {
        printf("object %d - inserting in retractable queue an event with timestamp %e\n", elem->destination,
               elem->timestamp);
        fflush(stdout);
    }
    // here we make a tail insert
    elem->prev = retractable_queue[dest].tail.prev;
    elem->next = &retractable_queue[dest].tail;
    retractable_queue[dest].tail.prev->next = elem;
    retractable_queue[dest].tail.prev = elem;

    __sync_fetch_and_add(&retractable_events, 1);
}

void rollback_speculation_queue(int object, double rollback_time) {
    fallback_slot *q = &speculation_queue[object];
    queue_elem *cur;
    queue_elem *prev;

    AUDIT {
        printf("object %d - rollback_speculation_queue called with rollback_time %e\n", object, rollback_time);
        fflush(stdout);
    }

    if (q->head == NULL) {
        return;
    }

    cur = q->tail;

    /* walk backwards removing elements >= rollback_time */
    while (cur && cur->send_time >= rollback_time) {
        prev = cur->prev;
        free(container_of(cur, event, q));
        cur = prev;
        __sync_fetch_and_add(&speculation_events, -1);
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

void queue_elem_annihilation(queue_elem *the_elem) {
    int object = the_elem->destination;
    int source;
    double cancellation_time = the_elem->timestamp;
    source = get_current();
    AUDIT {
        printf("queue_elem_annihilation for object %d with cancellation_time %e\n", object, cancellation_time);
        fflush(stdout);
    }
    object_lock(object);
    if (speculation[object].current_time >= cancellation_time && !speculation[object].standing_rollback) {
        speculation[object].causality_violation_time = cancellation_time;
        speculation[object].standing_rollback = 1;
        goto try_get_object;
    }
    if (speculation[object].current_time >= cancellation_time && speculation[object].standing_rollback) {
        if (speculation[object].causality_violation_time > cancellation_time) {
            speculation[object].causality_violation_time = cancellation_time;
            goto try_get_object;
        }
    }
try_get_object:
    if (speculation[object].the_state == FREE) {
        speculation[object].the_state = BUSY;
        if (speculation[object].in_stack == 0) {
            put_into_stack(object);
            speculation[object].in_stack = 1;
            if (!speculation[source].in_stack) {
                put_head_into_stack(source);
                speculation[source].in_stack = 1;
            }
        } else {
            printf("ERROR: object %d FREE but present in stack\n", object);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }

    // now really remove the event to be annihilated
    pthread_spin_lock(&locks[object][my_index].lock);
    the_elem->prev->next = the_elem->next;
    the_elem->next->prev = the_elem->prev;
    pthread_spin_unlock(&locks[object][my_index].lock);
    free(container_of(the_elem, event, q));
    object_unlock(object);
}

void log_rollback(int object, double rollback_time) {
    log_element *queue = &log_queue[object];
    log_element *curr = queue->last;
    log_element *prev;

    {
        printf("object %d - log_rollback called with rollback_time %e\n", object, rollback_time);
        fflush(stdout);
    }

    while (curr != NULL && curr->send_time >= rollback_time) {
        prev = curr->prev;

        /* unlink curr from the queue */
        if (curr->prev) {
            curr->prev->next = curr->next;
        } else {
            queue->first = curr->next;
        }

        if (curr->next) {
            curr->next->prev = curr->prev;
        } else {
            queue->last = curr->prev;
        }

        queue_elem_annihilation(curr->the_element);

        free(curr);
        curr = prev;
    }
}

void restore_retractable_events(int object) {
    queue_elem *head = &retractable_queue[object].head;
    queue_elem *tail = &retractable_queue[object].tail;
    queue_elem *current;
    while (head->next != tail) {
        current = head->next;
        head->next = current->next;
        current->next->prev = head;
        current->next = NULL;
        current->prev = NULL;
        __sync_fetch_and_sub(&retractable_events, 1);
        queue_insert(current);
        printf("Queues status during restore_retractable_events\n");
        print_queues_status(object);
    }
}

#endif
