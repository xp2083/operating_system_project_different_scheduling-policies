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
#define randSize 9999 

int lineNum = 0;

struct Process{
	int num;
	int cpu_all_time;
	int cpu_max;
	int io_max;
	int static_prio;
	int prio;
	int state_ts;
	int state_dura;
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

class Scheduler {
public:
	int quantum;
	deque<Process*> run_queue;
	virtual ~Scheduler(){};
	virtual void add_to_queue (Process* proc);
	virtual Process* get_from_queue ();
	virtual int get_quantum ();
	virtual void set_quantum (int num);
	int get_queue_size () {
		return run_queue.size();
	}
};

void Scheduler::add_to_queue (Process* proc){
}

Process* Scheduler::get_from_queue (){
	return NULL;
}

int Scheduler::get_quantum(){
	return 0;
}

void Scheduler::set_quantum(int num){

}

Scheduler* sched;  // that's the only object we use in global algo
int rand_cnt = 0; 

class Scheduler_fcfs: public Scheduler {
	public:
	~Scheduler_fcfs() {};
	void add_to_queue (Process* proc);
        Process* get_from_queue ();
	int get_quantum ();
	void set_quantum (int num);
	int get_queue_size ();
};

void Scheduler_fcfs::add_to_queue(Process* proc) {
	run_queue.push_back(proc);
	//printf("in add_to_queue size of sched queue %d\n", sched->get_queue_size());
	//printf("%s Q=%d\n", __FUNCTION__, obj->run_queue);
}

Process* Scheduler_fcfs::get_from_queue() {
	//printf("%s\n", __FUNCTION__);
	if (run_queue.size() != 0){
		Process* tmp = run_queue.front();
		run_queue.pop_front();
		return tmp;
	}
	return NULL;
}

int Scheduler_fcfs::get_quantum() {
        return quantum;
}

void Scheduler_fcfs::set_quantum(int num) {
        quantum = num;
}

//Scheduler* sched;  // that's the only object we use in global algo
//int rand_cnt = 0;

int my_random(int burst, long rand_num[randSize]){
	 int rand_res = (rand_num[rand_cnt] % burst) + 1;
	 //printf("rand_num %lu\n", rand_num[rand_cnt]);
	 //printf("rand_res %d\n", rand_res);
	 rand_cnt++;
	 return rand_res;
	  
}

Event* get_event(deque<Event>* event_queue){
	if (event_queue->size() > 0 ){
		Event* tmp = &(event_queue->front());
		event_queue->pop_front();
		return tmp;
	}else
		return NULL;
}

int insert_queue(deque<Event>* event_queue, Event eve){
	deque<Event>::iterator ite = event_queue->begin();
	if (event_queue->size() == 0){
		event_queue->push_back(eve);
	}
	else{
		while(eve.timestamp > ite->timestamp and ite != event_queue->end())
			ite++;
		if (eve.timestamp == ite->timestamp){
			while(eve.process->num > ite->process->num and ite != event_queue->end())
				ite++;
		}
		if (ite != event_queue->end())
			event_queue->insert(ite, eve);
		else
			event_queue->push_back(eve);
	}
	return 0;
}

int insert_info(vector<MidInfo>* info_vec, MidInfo info){
	vector<MidInfo>::iterator ite = info_vec->begin();
	if (info_vec->size() == 0){
		info_vec->push_back(info);
	}else{
		while (info.s_time > ite->s_time and ite != info_vec->end())
			ite++;
		if (ite != info_vec->end())
			info_vec->insert(ite, info);
		else
			info_vec->push_back(info);
	}
	return 0;
}

int put_event(deque<Event>* event_queue, Process* process, int old_state, long rand_num[randSize], int cur_time){
        Event event;
	event.process = process;
	event.old_state = old_state;
	if (event.old_state == STATE_RUNNING){
		event.new_state = STATE_BLOCK;
		event.transition = TRANS_TO_BLOCK;
		int tmp_cb = my_random(process->cpu_max, rand_num);
		if (process->cpu_all_time >= tmp_cb)
			event.timestamp = cur_time + tmp_cb;
		else
			event.timestamp = cur_time + process->cpu_all_time;		
	}
	else if (event.old_state == STATE_BLOCK){
		event.new_state	= STATE_READY;
		event.transition = TRANS_TO_READY;
		event.timestamp = cur_time + my_random(process->io_max, rand_num);;
	}
	insert_queue(event_queue, event);
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
			prev_state = "RUNNG";
			break;
		case STATE_BLOCK:
			prev_state = "BLOCK";
			break;
	}
	switch(info.next_state){
		case STATE_CREATED:
			next_state = "CREATED";
			break;
		case STATE_READY:
			next_state = "READY";
			break;
		case STATE_RUNNING:
			next_state = "RUNNG";
			break;
		case STATE_BLOCK:
			next_state = "BLOCK";
			break;
	}
	

	printf("%d %d %d:", info.s_time, info.process, info.last_time);
	if (info.rem == 0) 
		printf(" Done\n");
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

int get_next_event_time(deque<Event>* event_queue){
	if (&(event_queue->front()) != NULL)
		return (event_queue->front()).timestamp;
	else
		return -1;	

}

int simulation(ifstream* file, long rand_num [randSize], deque<Event>* event_queue){
	Event* evt;
	bool CALL_SCHEDULER = true;
	vector<MidInfo> info_vec; 
	Process* cur_proc = NULL; 
	while (evt = get_event(event_queue)){
		Process *proc = evt->process;
        	int cur_time = evt->timestamp;
		int transition = evt->transition;
		int timeInPrev = cur_time - proc->state_ts;
		//delete evt;
		evt = NULL;
	        MidInfo info;
	
		switch(transition){
			case TRANS_TO_READY:
				//must come from BLOCKED or CREATED 
				//add to run queue, no event created 
				CALL_SCHEDULER = true;
				sched->add_to_queue(proc);
				if (proc->state_prev == STATE_CREATED){
					//for info print
					proc->state_prev_prev = STATE_CREATED;
					proc->state_prev = STATE_READY;
					proc->state_ts = cur_time;
					proc->state_dura = timeInPrev; 
					
				} 
				else {
					//come from BLOCKED
					//create info for RUNNING->BLOCK	
					createInfo(&info, proc->state_ts, proc->num, proc->state_dura, proc->state_prev_prev, proc->state_prev, 0, timeInPrev, proc->cpu_all_time, proc->static_prio);
					//info_vec.push_back(info);
					insert_info(&info_vec, info);
					//printInfo(info);

				      	//for print info
					proc->state_prev_prev = STATE_BLOCK;
					proc->state_prev = STATE_READY;
					proc->state_ts = cur_time;
					proc->state_dura = timeInPrev;

				}
				break;
			case TRANS_TO_PREEMPT:
				//must come from RUNNING 
				//add to run queue, no event is generated
				CALL_SCHEDULER = true;
				break;
			case TRANS_TO_RUNNING:
				//comes from READY
				//create info for CREATED/BLOCK->READY
				createInfo(&info, proc->state_ts, proc->num, proc->state_dura, proc->state_prev_prev, proc->state_prev, 0, 0, proc->cpu_all_time, proc->prio);
				insert_info(&info_vec, info);
				//info_vec.push_back(info);
				//printInfo(info);

				//create event for next step, it is either preempt or block
				put_event(event_queue, proc, STATE_RUNNING, rand_num, cur_time);
				
				//set process state for print info
				proc->state_prev = STATE_RUNNING;
				proc->state_prev_prev = STATE_READY;
				proc->state_ts = cur_time; 
				proc->state_dura = timeInPrev;
				break;
			case TRANS_TO_BLOCK:
					//create info for READY->RUNNING 	
					createInfo(&info, proc->state_ts, proc->num, proc->state_dura, proc->state_prev_prev, proc->state_prev, timeInPrev, 0, proc->cpu_all_time, proc->prio);
					insert_info(&info_vec, info);
					//info_vec.push_back(info);
					//printInfo(info);
					CALL_SCHEDULER = true;
					if (proc->cpu_all_time - timeInPrev > 0){
						//create an event foo when process becomes READY again
						put_event(event_queue, proc, STATE_BLOCK, rand_num, cur_time);

						//set process state for next session
						proc->cpu_all_time = proc->cpu_all_time - timeInPrev;
						proc->state_prev = STATE_BLOCK;
						proc->state_prev_prev = STATE_RUNNING;
						proc->state_ts = cur_time;
						proc->state_dura = timeInPrev;
					}
					else{
						proc->cpu_all_time = proc->cpu_all_time - timeInPrev;
						createInfo(&info, cur_time, proc->num, timeInPrev, proc->state_prev_prev, proc->state_prev, timeInPrev, 0, proc->cpu_all_time, proc->prio);
						insert_info(&info_vec, info);
					        //info_vec.push_back(info);
					        //printInfo(info);
					}
					cur_proc = NULL;

				break;
		}
		if(CALL_SCHEDULER){
			if (get_next_event_time(event_queue) == cur_time)
			//process next event from event queue
				continue;
			CALL_SCHEDULER = false;
			if (cur_proc == NULL){
				cur_proc = sched->get_from_queue();
				if (cur_proc == NULL)
					continue;
				//create event to make this process runnable for same time?
				//the state will be STATE_RUNNING
				if (cur_proc->state_prev == STATE_READY){
					Event event;
					event.timestamp = cur_proc->state_ts;
					event.process = cur_proc;
					event.old_state = STATE_READY;
					event.new_state = STATE_RUNNING;
					event.transition = TRANS_TO_RUNNING;
					insert_queue(event_queue, event);
				}
			}
		}
	}
	vector<MidInfo>::iterator ite = info_vec.begin();
	while (ite != info_vec.end()){
		printInfo(*ite);
		ite++;
	} 	
}

int init_event_proc(ifstream* file, deque<Event>* event_queue){
	while (file->peek() != EOF){
		//create new process and add to scheduler process deque
		char tmp [maxVecSize] = {0};
		file->getline(tmp, maxVecSize); 			
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
		proc->state_dura = 0;
		proc->state_prev = STATE_CREATED;
		proc->state_prev_prev = STATE_CREATED;
		proc->static_prio = 2;
		proc->prio = 1;
		proc->num = lineNum;
		lineNum++;

		//create new event and add to event queue 
		Event event;
		event.timestamp = proc->state_ts;
		event.process = proc;
		event.old_state = STATE_CREATED;
		event.new_state = STATE_READY;
		event.transition = TRANS_TO_READY;
		event_queue->push_back(event);					 

	}
	return 0;	
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
		{
	    Scheduler_fcfs* fcfs_scheduler = new Scheduler_fcfs();  
	    fcfs_scheduler->set_quantum(1000000);
            sched = fcfs_scheduler;
            break;
		}
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
        //initialize all process and their created->ready event 
 	deque<Event> event_queue;       
	init_event_proc(&file, &event_queue);
	
        //open rand file 
        char rand_file_path [vecSize]= "./rfile";
	ifstream rand_file(rand_file_path);
	if (!rand_file.is_open()){
		printf("no open\n");
		return -1;	
	}

	long rand_num [randSize];
	int cnt = 0;
	while (rand_file.peek() != EOF){		
		char tmp [16] = {0};
        	rand_file.getline(tmp, sizeof(tmp));	
        	rand_num[cnt] = atol(tmp);	
		cnt++;
	}
	
	simulation(&file, rand_num, &event_queue);

	return 0; 	


	
}

