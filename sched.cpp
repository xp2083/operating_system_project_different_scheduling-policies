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
#include <deque>
using namespace std;

typedef enum {STATE_CREATED, STATE_READY, STATE_RUNNING , STATE_BLOCKED } process_state_t;
typedef enum {TRANS_TO_READY, TRANS_TO_PREEMPT, TRANS_TO_RUN, TRANS_TO_BLOCK} procee_strans_state;

#define maxVecSize 512; 

//“int myrandom(int burst) { return 1 + (randvals[ofs] % burst); }”
int lineNum = 0;

struct Process{
	int num;
	int all_cpu_time;
	int cpu_max;
	int io_max;
	int static_prio;
	int state_ts;
	int state;
};

struct Event{
	int timestamp;
	process* process;
	int old_state;
	int new_state;
	int transition;	
	int cb;
	int ib;
	int rem;
	int prio;
};

struct MidInfo{
	int s_time;
	int process;
	int last_time;
	int prev_state;
	int next_state;
	int cb;
	int ib;
	int rem;
	int prio;
};

struct scheduler; 

struct function_table {
	void (*add_to_queue) (struct scheduler *obj, deque<Process*>* queue);
	process* (*get_from_queue) (struct scheduler *obj, deque<Process*>* queue);
	int (*get_quantum) (struct scheduler *obj);	     
};

struct scheduler {
	struct function_table* funcs;
	deque<Process*>* run_queue;
};

struct scheduler_fcfs{
	struct scheduler base;
	int quantum;
}

Process* fcfs_get_from_queue(struct scheduler* obj, deque<Process*>* run_queue, ifstream* file) {
        struct scheduler_fcfs *sobj = (struct scheduler_fcfs*)obj; 
        //printf("%s\n", __FUNCTION__);
        if (run_queue.size()! = 0)
		return run_queue.pop_front();
	else{
		char tmp [maxVecSize] = {0};
		file.getline(tmp, sizeof(tmp)); 			
		const char *delim = " ";
		Process* proc;
		char* token = strtok(*tmp, delim);
		proc->state_ts = atoi(token);
		int cnt = 0;
		while (!token != NULL){
			token = strtok(NULL, delim);
			if (cnt == 0){
				proc->all_cpu_time = atoi(token);
			}
			else if (cnt == 1){
				proc->cpu_max = atoi(token);
			}
			else if (cnt == 2){
				proc->io_max = atoi(token);
			}
			cnt++;
		} 	
		proc->state = STATE_READY;
	        proc->num = lineNum;
		lineNum++;	
		return proc;			
	}
	return 0;
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

int myrandom(int burst){
	 return 1 + (randvals[ofs] % burst); 
}

Event* get_event(deque<Event>* event_queue){
	return &event_queue->pop_front();
}

int createInfo (MidInfo* info, int start_time, int processNum, int last_time, int prev_state, int next_state,int cb, int ib, int rem, int prio) {
	info->s_time = start_time;
	info->process = processNum;
 	info->last_time = last_time;
	info->prev_state = prev_state;
	info->next_state = next_state;
	info->cb = cb;
	info->ib = ib;
	info->rem = rem;
	info->prio = prio;

	return 0;

}

int simulation(ifstream* file){
	Event* evt;
	bool CALL_SCHEDULER = true;
	deque<Process*>* run_queue;
	deque<Event>* event_queue;
	vector<MidInfo>* info_vec; 
	Process* cur_proc; 
	while (evt = get_event()){
		Process *proc = evt->process;
        	int cur_time = evt->timestamp;
		int transition = evt->transition;
		int timeInPrev = cur_time - proc->state_ts;
		delete evt;
		evt = NULL;
		
		switch(transition){
			case TRANS_TO_READY:
			//must come from BLOCKED or CREATED 
			//add to run queue, no event created 
				CALL_SCHEDULER = true;
				run_queue -> push_back(proc);
				MidInfo info;
				createInfo(&info, proc->stat_ts, proc->num, timeInPrev, )
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
			if (cur_proc == NULL){
				cur_proc = sched->funcs->get_from_queue(sched, run_queue);
				if (cur_proc == null)
					continue;
				//create event to make this process runnable for same time
				if (cur_proc -> state == STATE_CREATED){
					Event event;
					event.timestamp = cur_proc->state_ts;
					event.process = cur_proc;
					event.old_state = STATE_CREATED;
					event.new_state = STATE_READY;
					event.transition = TRANS_TO_READY;

					event_queue -> push_back(event);					 
				}
			}
		}
		

	}

}


int main (int argc, char* argv)
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

	char* filePath = argv[1];
	ifstream file(filePath);
	if (!file.is_open()){
		printf("no open\n");
		return -1;	
	}	

	Simulaton(ifstream* file);

	return 0; 	


	
}

