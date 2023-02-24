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

typedef enum {STATE_CREATED=1, STATE_READY=2, STATE_RUNNING=3, STATE_BLOCK=4} process_state_t;
typedef enum {TRANS_TO_READY=1, TRANS_TO_RUNNING=2, TRANS_TO_BLOCK=3, TRANS_TO_PREEMPT=4} procee_strans_state;

#define vecSize 64
#define maxVecSize 512 

int lineNum = 0;

struct Process{
	int num;
	int cpu_all_time;
	int cpu_max;
	int io_max;
	int static_prio;
	int state_ts;
	int state;
	int state_prev;
	int state_prev_prev; 
};

struct Event{
	int timestamp;
	Process* process;
	int old_state;
	int new_state;
	int transition;	
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
	void (*add_to_queue) (struct scheduler *obj, deque<Process*>* queue, ifstream* file);
	Process* (*get_from_queue) (struct scheduler *obj, deque<Process*>* queue, ifstream* file);
	int (*get_quantum) (struct scheduler *obj);	     
};

struct scheduler {
	struct function_table* funcs;
	deque<Process*>* run_queue;
};

struct scheduler_fcfs{
	struct scheduler base;
	int quantum;
};

/*
Process* fcfs_get_from_queue(struct scheduler* obj, deque<Process*>* run_queue, ifstream* file) {
	struct scheduler_fcfs *sobj = (struct scheduler_fcfs*)obj; 
	//printf("%s\n", __FUNCTION__);
	if (run_queue->size() != 0){
		Process* tmp = run_queue->front();
		run_queue->pop_front();
		return tmp;
	}
	else{
		add_to_queue(obj, run_queue, file);	
		Process* tmp = run_queue->front();
		run_queue->pop_front();
		return tmp;			
	}
	return 0;
}*/

void fcfs_add_to_queue(struct scheduler* obj, deque<Process*>* run_queue, ifstream* file) {
	struct scheduler_fcfs *sobj = (struct scheduler_fcfs*)obj;
	char tmp [maxVecSize] = {0};
	file->getline(tmp, sizeof(tmp)); 			
	const char *delim = " ";
	Process* proc = (struct Process*)malloc(sizeof(struct Process));
	char* token = strtok(tmp, delim);
	proc->state_ts = atoi(token);
	int cnt = 0;
	while (token != NULL){
		token = strtok(NULL, delim);
		if (cnt == 0){
			proc->cpu_all_time = atoi(token);
		}
		else if (cnt == 1){
			proc->cpu_max = atoi(token);
		}
		else if (cnt == 2){
			proc->io_max = atoi(token);
		}
		cnt++;
	} 	
	proc->state = STATE_CREATED;
	proc->num = lineNum;
	lineNum++;
        run_queue->push_back(proc);		

	//printf("%s Q=%d\n", __FUNCTION__, obj->run_queue);
}
Process* fcfs_get_from_queue(struct scheduler* obj, deque<Process*>* run_queue, ifstream* file) {
	struct scheduler_fcfs *sobj = (struct scheduler_fcfs*)obj; 
	//printf("%s\n", __FUNCTION__);
	if (run_queue->size() != 0){
		Process* tmp = run_queue->front();
		run_queue->pop_front();
		return tmp;
	}
	else{
		fcfs_add_to_queue(obj, run_queue, file);	
		Process* tmp = run_queue->front();
		run_queue->pop_front();
		return tmp;			
	}
	return 0;
}

int fcfs_get_quantum(struct scheduler* obj) {
        return ((struct scheduler_fcfs*)obj)->quantum;
}

struct function_table fcfs_functions = {
	.add_to_queue = &fcfs_add_to_queue,
        .get_from_queue = &fcfs_get_from_queue,
        .get_quantum = &fcfs_get_quantum
};

struct scheduler* sched;  // that's the only object we use in global algo
struct scheduler_fcfs *fcfs_scheduler; // this is specialized

int my_random(int burst, int rand_num){
	 return 1 + (rand_num % burst); 
}

Event* get_event(deque<Event>* event_queue){
	Event* tmp = &(event_queue->front());
	event_queue->pop_front();
	return tmp;
}

int put_event(deque<Event>* event_queue, Process* process, int old_state, int rand_num){
        Event event;
	event.process = process;
	event.old_state = old_state;
	if (event.old_state == STATE_RUNNING){
		event.new_state = STATE_BLOCK;
		event.transition = TRANS_TO_BLOCK;
		event.timestamp = process->state_ts + my_random(process->io_max, rand_num); 
	}
	else if (event.old_state == STATE_BLOCK){
		event.new_state	= STATE_READY;
		event.transition = TRANS_TO_READY;
		event.timestamp = process->state_ts;
	}	
}

int createInfo (MidInfo* info, int start_time, int process_num, int last_time, int prev_state, int next_state,int cb, int ib, int rem, int prio) {
	info->s_time = start_time;
	info->process = process_num;
 	info->last_time = last_time;
	info->prev_state = prev_state;
	info->next_state = next_state;
	info->cb = cb;
	info->ib = ib;
	info->rem = rem;
	info->prio = prio;

	return 0;
}


void printInfo(MidInfo info){
	string prev_state = "";
	string next_state = "";
	switch(info.prev_state){
		case STATE_CREATED:
			prev_state = "CREATED";
			break;
		case STATE_READY:
			prev_state = "READY";
			break;
		case STATE_RUNNING:
			prev_state = "RUNNING";
			break;
		case STATE_BLOCK:
			prev_state = "RUNNING";
			break;
	}
	switch(info.next_state){
		case STATE_CREATED:
			prev_state = "CREATED";
			break;
		case STATE_READY:
			prev_state = "READY";
			break;
		case STATE_RUNNING:
			prev_state = "RUNNING";
			break;
		case STATE_BLOCK:
			prev_state = "RUNNING";
			break;
	}
	

	printf("%d %d %d: ", info.s_time, info.process, info.last_time);
	if (info.rem == 0) 
		printf("Done\n");
	else{
		printf(" %s -> %s", prev_state.c_str(), next_state.c_str());
	if (info.prev_state == STATE_CREATED | info.prev_state == STATE_BLOCK)
		printf("\n");
	else if (info.prev_state == STATE_READY)
		printf(" cb=%d rem=%d prio=%d\n", info.cb, info.rem, info.prio);
	else if (info.prev_state == STATE_RUNNING)
		printf(" ib=%d rem=%d\n", info.ib, info.rem);
	}	
}

int simulation(ifstream* file, int rand_num){
	Event* evt;
	bool CALL_SCHEDULER = true;
	deque<Process*> run_queue;
	deque<Event> event_queue;
	vector<MidInfo> info_vec; 
	Process* cur_proc; 
        //initialization
				cur_proc = sched->funcs->get_from_queue(sched, &run_queue, file);
				if (cur_proc == NULL)
					return -1;
				//create event to make this process runnable for same time
				if (cur_proc->state == STATE_CREATED){
					Event event;
					event.timestamp = cur_proc->state_ts;
					event.process = cur_proc;
					event.old_state = STATE_CREATED;
					event.new_state = STATE_READY;
					event.transition = TRANS_TO_READY;
					event_queue.push_back(event);	
				}
				else {
				//the state will be STATE_RUNNING
					Event event;
					event.timestamp = cur_proc->state_ts;
					event.process = cur_proc;
					event.old_state = STATE_READY;
					event.new_state = STATE_RUNNING;
					event.transition = TRANS_TO_RUNNING;
					event_queue.push_back(event);					 
				}
	while (evt = get_event(&event_queue)){
		Process *proc = evt->process;
        	int cur_time = evt->timestamp;
		int transition = evt->transition;
		int timeInPrev = cur_time - proc->state_ts;
		delete evt;
		evt = NULL;
	        MidInfo info;
	
		switch(transition){
			case TRANS_TO_READY:
			//must come from BLOCKED or CREATED 
			//add to run queue, no event created 
			 	CALL_SCHEDULER = true;
				if (proc->state == STATE_CREATED){
				//come from CREATED
				proc->state_prev_prev = STATE_CREATED;
				proc->state_prev = STATE_READY;
				proc->state = STATE_RUNNING;
				run_queue.push_back(proc);
				} 
				else {
				//come from BLOCKED
		         	//create the info for RUNNING->BLOCKED					      
				createInfo(&info, cur_time, proc->num, timeInPrev, proc->state_prev, proc->state_prev_prev,0,timeInPrev,proc->cpu_all_time-timeInPrev, proc->static_prio);
				info_vec.push_back(info);
 				printInfo(info);
				proc->cpu_all_time = proc->cpu_all_time-timeInPrev;
				}
				break;
			case TRANS_TO_PREEMPT:
			//must come from RUNNING 
			//add to run queue, no event is generated
				CALL_SCHEDULER = true;
				break;
			case TRANS_TO_RUNNING:
			//create event for either preempt or block
				put_event(&event_queue, proc, STATE_RUNNING, rand_num); 
				//create the info for CREATED->READY/BLOCKED->READY 
				createInfo(&info, cur_time, proc->num, timeInPrev, proc->state_prev, proc->state_prev_prev, 0, 0, proc->cpu_all_time, 0);
				info_vec.push_back(info);
 				printInfo(info);
				break;
			case TRANS_TO_BLOCK:
			//create an event foo when process becomes READY again
				put_event(&event_queue, proc, STATE_BLOCK, rand_num);
				CALL_SCHEDULER = true;
				//create the info for READY->RUN
				createInfo(&info, cur_time, proc->num, timeInPrev, proc->state_prev, proc->state_prev_prev, timeInPrev, 0, proc->cpu_all_time, proc->static_prio);
				info_vec.push_back(info);
 				printInfo(info);
				break;
			}
		if(CALL_SCHEDULER){
			//if (get_next_event_time() == cur_time)
			//process next event from event queue
			//	continue;
			CALL_SCHEDULER = false;
			if (cur_proc == NULL){
				cur_proc = sched->funcs->get_from_queue(sched, &run_queue, file);
				if (cur_proc == NULL)
					continue;
				//create event to make this process runnable for same time?
				//the state will be STATE_RUNNING
					Event event;
					event.timestamp = cur_proc->state_ts;
					event.process = cur_proc;
					event.old_state = STATE_READY;
					event.new_state = STATE_RUNNING;
					event.transition = TRANS_TO_RUNNING;
					event_queue.push_back(event);					 
			}
		}
	}
}


int main (int argc, char* argv[])
{
	//create scheduler
	int c;
	int schedtype = 1;

        /*
	while ((c = getopt(argc,argv,"s:")) != -1 )
        {
                switch(c) {
                case 's':
                        schedtype = atoi(optarg);
                        break;
                }
        }*/

        switch (schedtype) {
        case 1:
	    	
            fcfs_scheduler = (struct scheduler_fcfs*)malloc(sizeof(struct scheduler_fcfs));
            fcfs_scheduler->base.funcs = &fcfs_functions;
            fcfs_scheduler->quantum = 100000;
            sched = (struct scheduler*)fcfs_scheduler;
            break;
        case 2:
            break;
        default:
            printf("At least specify a valid scheduler\n");
            exit(1);
        }

        //open input file
	char* file_path = argv[1];
	ifstream file(file_path);
	if (!file.is_open()){
		printf("no open\n");
		return -1;	
	}	
        //open rand file 
        char rand_file_path [vecSize]= "./rfile";
	ifstream rand_file(rand_file_path);
	if (!rand_file.is_open()){
		printf("no open\n");
		return -1;	
	}
	char tmp [vecSize] = {0};
        rand_file.getline(tmp, sizeof(tmp));	
        int rand_num = atoi(tmp);	

	simulation(&file, rand_num);

	return 0; 	


	
}

