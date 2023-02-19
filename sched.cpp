#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <sstream>
#include <iostream>
#include <climits>
using namespace std;

typedef enum {STATE_CREATED, STATE_RUNNING , STATE_BLOCKED } process_state_t;
typedef enum {TRANS_TO_READY, TRANS_TO_PREEMPT, TRANS_TO_RUN, TRANS_TO_BLOCK} procee_strans_state;

//“int myrandom(int burst) { return 1 + (randvals[ofs] % burst); }”

struct process{
	int id;
	int arr_time;
	int cpu_time;
	int cpu_burst;
	int io_burst;
	int static_prio;
}

struct scheduler; 

struct function_table {
	void (*add_to_queue) (struct scheduler *obj, deque* queue);
	process* (*get_from_queue) (struct scheduler *obj, deque* queue);
	int (*get_quantum) (struct scheduler *obj);	     
};

struct scheduler {
	struct function_table* funcs;
	deque* run_queue;
};

struct scheduler_fcfs{
	struct scheduler base;
	int quantum;
}

int fcfs_get_from_queue(struct scheduler* obj, int a) {
        struct scheduler_fcfs *sobj = (struct scheduler_fcfs*)obj; // typo coercion
        printf("%s\n", __FUNCTION__);
        return 1;
}

void fcfs_add_to_queue(struct scheduler* obj, int a) {
        struct scheduler_fcfs *sobj = (struct scheduler_fcfs*)obj;
        printf("%s Q=%d\n", __FUNCTION__, obj->ready_queue);
}

int fcfs_get_quantum(struct scheduler* obj) {
        return ((struct scheduler_fcfs*)obj)->quantum;
}

struct function_table fcfs_functions = {
        .get_from_queue = &fcfs_get_from_queue,
        .add_to_queue = &fcfs_add_to_queue,
        .get_quantum = &fcfs_get_quantum
};

struct scheduler* sched;  // that's the only object we use in global algo
struct scheduler_fcfs *fcfs_scheduler; // this is specialized

int simulation(){
	Event* evt;
	bool CALL_SCHEDULER = false;
	while (evt = get_event()){
		Process *proc = evt->process;
        	int cur_time = evt->timeStamp;
		int transition = evt->transition;
		int timeInPrev = cur_time - proc->state_ts;
		delete evt;
		evt = NULL;
		
		switch(transition){
			case TRANS_TO_READY:
			//must come from BLOCKED or CREATED 
			//add to run queue, no event created 
				CALL_SCHEDULER = true;
				break;
			case TRANS_TO_PREEMPT:
			//must come from RUNNING 
			//add to run queue, no event is generated
				CALL_SCHEDULER = true;
				break;
			case TRANS_TO_RUN:
			//create event for either preempt or block
				break;
			case TRANS_TO_BLOCK:
			//create an event fro when process becomes READY again
				CALL_SCHEDULER = true;
				break;
			}
		if(CALL_SCHEDULER){
			if (get_next_event_time() == cur_time)
			//process next event from event queue
				continue;
			CALL_SCHEDULER = false;
			if (CURRENT_RUNNING_PROCESS == NULL){
				CURRENT_RUNNING_PROCESS = sched->funcs->get_from_queue(sched, deque));
				if (CURRENT_RUNNING_PROCESS == null)
					continue;
				//create event to make this process runnable fro same time
				//

			}
		}
		

	}

}


int main (int argc, char** argv)
{
	//create scheduler
	int c;
	int schedtype = 0;
        
	while ((c = getopt(argc,argv,"s:")) != -1 )
        {
                switch(c) {
                case 's':
                        schedtype = atoi(optarg);
                        break;
                }
        }

        switch (schedtype) {
        case 1:
            fcfs_scheduler = (struct scheduler_fcfs*)malloc(sizeof(struct scheduler_fcfs));
            fcfs_scheduler->base.funcs = &fcfs_functions;
            fcfs_scheduler->base.ready_queue = ?;
            fcfs_scheduler->quantum = 100000;
            sched = (struct scheduler*)fcfs_scheduler;
            break;
        case 2:
            break;
        default:
            printf("At least specify a valid scheduler\n");
            exit(1);
        }

	Simulaton();

	return 0; 	


	
}

