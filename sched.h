
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
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
#define DEBUG 1
#define defLimit 4
#define maxQuan 1000000
#define maxPrioDef 4

struct Process{
	int num;
	int cpu_all_time;
	int cpu_max;
	int io_max;
	int static_prio;
	int prio;
	int state_ts;
	int state_dura;
	int state;
	int state_prev;
	int state_prev_prev;
	int left_cb;
	int start_time;
	int total_cpu;
	int finish_time;
	int run_time;
	int io_time;
	int cpu_wait_time;
	int cpu_utiliz_time;
	int last_ib_start;
	int last_ib_end;
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

int insert_queue(deque<Event>* event_queue, Event eve);

class Scheduler {
protected:
	int quantum;
	deque<Process*> run_queue;
	        int max_prio;
        vector< deque<Process*> > run_queue_list;
        vector< deque<Process*> > expire_queue_list;
public:	
	virtual ~Scheduler(){};
	virtual void add_to_queue (Process* proc);
	virtual Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
	virtual int get_quantum ();
	virtual void set_quantum (int num);
};

void Scheduler::add_to_queue (Process* proc){
}

Process* Scheduler::get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time){
	return NULL;
}

int Scheduler::get_quantum(){
	return 0;
}

void Scheduler::set_quantum(int num){

}

class Scheduler_FCFS: public Scheduler {
	public:
	~Scheduler_FCFS() {};
	void add_to_queue (Process* proc);
        Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
	int get_quantum ();
	void set_quantum (int num);
};

void Scheduler_FCFS::add_to_queue(Process* proc) {
	run_queue.push_back(proc);
}

Process* Scheduler_FCFS::get_from_queue(Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time) {
	if (cur_proc != NULL)
		return cur_proc;
	if (run_queue.size() != 0){
		Process* tmp = run_queue.front();
		run_queue.pop_front();
		return tmp;
	}
	return NULL;
}

int Scheduler_FCFS::get_quantum() {
        return quantum;
}

void Scheduler_FCFS::set_quantum(int num) {
        quantum = num;
}


class Scheduler_LCFS: public Scheduler {
	public:
	~Scheduler_LCFS() {};
	void add_to_queue (Process* proc);
        Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
	int get_quantum ();
	void set_quantum (int num);
};

void Scheduler_LCFS::add_to_queue(Process* proc) {
	run_queue.push_back(proc);
}

Process* Scheduler_LCFS::get_from_queue(Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time) {
	if (cur_proc != NULL)
		return cur_proc;
	if (run_queue.size() != 0){
		Process* tmp = run_queue.back();
		run_queue.pop_back();
		return tmp;
	}
	return NULL;
}

int Scheduler_LCFS::get_quantum() {
        return quantum;
}

void Scheduler_LCFS::set_quantum(int num) {
        quantum = num;
}

class Scheduler_SRTF: public Scheduler {
	public:
	~Scheduler_SRTF() {};
	void add_to_queue (Process* proc);
	Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
	int get_quantum ();
	void set_quantum (int num);
};

void Scheduler_SRTF::add_to_queue(Process* proc) {
	deque<Process*>::iterator ite = run_queue.begin();
	if(run_queue.size() == 0)
		run_queue.push_back(proc);
	else{
		while(ite != run_queue.end()){
		if (proc->cpu_all_time >= (*ite)->cpu_all_time)
		//add = means if there is a same remaining time process, 
		//the new process should stays behind it
			ite++;
		else
			break;
		}
		if (ite != run_queue.end())
			run_queue.insert(ite, proc);
		else	 	
			run_queue.push_back(proc);
	}
}

Process* Scheduler_SRTF::get_from_queue(Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time) {
	if (cur_proc != NULL)
		return cur_proc;
	if (run_queue.size() != 0){
		Process* tmp = run_queue.front();
		run_queue.pop_front();
		return tmp;
	}
	return NULL;
}

int Scheduler_SRTF::get_quantum() {
        return quantum;
}

void Scheduler_SRTF::set_quantum(int num) {
        quantum = num;
}

class Scheduler_RR: public Scheduler {
	public:
	Scheduler_RR() {
	};
	~Scheduler_RR() {};
	void add_to_queue (Process* proc);
        Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
	int get_quantum ();
	void set_quantum (int num);
};

void Scheduler_RR::add_to_queue(Process* proc) {
	proc->prio = proc->static_prio-1;
	run_queue.push_back(proc);
}

Process* Scheduler_RR::get_from_queue(Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time) {
	if (cur_proc != NULL)
		return cur_proc;
	if (run_queue.size() != 0){
		Process* tmp = run_queue.front();
		run_queue.pop_front();
		return tmp;
	}
	return NULL;
}

int Scheduler_RR::get_quantum() {
        return quantum;
}

void Scheduler_RR::set_quantum(int num) {
        quantum = num;
}

class Scheduler_PRIO: public Scheduler {
	public:
	Scheduler_PRIO(int prio);
	~Scheduler_PRIO() {};
	void add_to_queue (Process* proc);
        Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
	int get_quantum ();
	void set_quantum (int num);
};

Scheduler_PRIO::Scheduler_PRIO(int prio){
	max_prio = prio;
	for (int i=0; i < max_prio; i++){
		deque<Process*> run_queue;
		deque<Process*> expire_queue; 
		run_queue_list.push_back(run_queue);
		expire_queue_list.push_back(expire_queue);
	}
}

void Scheduler_PRIO::add_to_queue(Process* proc) {
	if (proc->prio == -1){
		proc->prio = proc->static_prio - 1;
		(expire_queue_list[proc->prio]).push_back(proc);
	}
	else
		(run_queue_list[proc->prio]).push_back(proc);	
}

Process* Scheduler_PRIO::get_from_queue(Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time) {
	if (cur_proc != NULL)
		return cur_proc;
	Process* tmp = NULL;
	for (int i=max_prio-1; i >=0; i--){
		if ((run_queue_list[i]).size() > 0){
			tmp = run_queue_list[i].front();
			run_queue_list[i].pop_front();
			break;
		}
	}
	if (tmp == NULL){
		for(int i=max_prio-1; i >=0; i--){
			if ((run_queue_list[i]).size() == 0){
				deque<Process*> tt = run_queue_list[i];
				run_queue_list[i] = expire_queue_list[i];
				expire_queue_list[i] = tt;

	                }
	}
		for (int i=max_prio-1; i >=0; i--){
		if ((run_queue_list[i]).size() > 0){
			tmp = run_queue_list[i].front();
			run_queue_list[i].pop_front();
			break;
		}
	}
			
	}
	return tmp;
}

int Scheduler_PRIO::get_quantum() {
        return quantum;
}

void Scheduler_PRIO::set_quantum(int num) {
        quantum = num;
}

class Scheduler_EPRIO: public Scheduler {
	public:
	Scheduler_EPRIO(int prio);
	~Scheduler_EPRIO() {};
	void add_to_queue (Process* proc);
        Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
	int get_quantum ();
	void set_quantum (int num);
};

Scheduler_EPRIO::Scheduler_EPRIO(int prio){
	max_prio = prio;
	for (int i=0; i < max_prio; i++){
		deque<Process*> run_queue;
		deque<Process*> expire_queue; 
		run_queue_list.push_back(run_queue);
		expire_queue_list.push_back(expire_queue);
	}
}

void Scheduler_EPRIO::add_to_queue(Process* proc) {
	if (proc->prio == -1){
		proc->prio = proc->static_prio - 1;
		(expire_queue_list[proc->prio]).push_back(proc);
	}
	else
		(run_queue_list[proc->prio]).push_back(proc);	
}

Process* Scheduler_EPRIO::get_from_queue(Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time) {
	Process* tmp = NULL;
	if (cur_proc == NULL){
		for (int i=max_prio-1; i >=0; i--){
			if ((run_queue_list[i]).size() > 0){
				tmp = run_queue_list[i].front();
				run_queue_list[i].pop_front();
				break;
			}
		}
		if (tmp == NULL){
			for(int i=max_prio-1; i >=0; i--){
				if ((run_queue_list[i]).size() == 0){
					deque<Process*> tt = run_queue_list[i];
					run_queue_list[i] = expire_queue_list[i];
					expire_queue_list[i] = tt;

				}
			}
			for (int i=max_prio-1; i >=0; i--){
				if ((run_queue_list[i]).size() > 0){
					tmp = run_queue_list[i].front();
					run_queue_list[i].pop_front();
					break;
				}
			}		
		}
	}
	else{
		tmp = cur_proc;	
		for (int i=max_prio-1; i >=0; i--){
			if ((run_queue_list[i]).size() > 0){
				if (cur_proc->prio < i){
					//find higher priority process
					tmp = run_queue_list[i].front();
					break;
				}
			}
		}
		if (tmp != cur_proc){
			//find the original event for block
			//change it to preempt
                        deque<Event>::iterator ite = event_queue->begin();
			while(ite != event_queue->end()){ 
				if (ite->process != cur_proc)
					ite++;
				else
					break; 
			}
			Event evt = (*ite);
			event_queue->erase(ite);
			evt.old_state = STATE_RUNNING;
			evt.new_state = STATE_READY;
			evt.transition = TRANS_TO_PREEMPT;
			evt.timestamp = cur_time;
			insert_queue(event_queue, evt);

			cur_proc->left_cb = (*cur_end_time) + cur_proc->left_cb - cur_time;
			cur_proc->state_prev_prev = STATE_READY;
			cur_proc->state_prev = STATE_RUNNING;
			cur_proc->state = STATE_READY;
			(*cur_end_time) = cur_time;

		}
	}
	return tmp;
}

int Scheduler_EPRIO::get_quantum() {
        return quantum;
}

void Scheduler_EPRIO::set_quantum(int num) {
 quantum = num;
}
///////////////////////////////////////


int my_random(int up_limit, vector<long>* rand_num, vector<long>::iterator* rand_ite);

Event* get_event(deque<Event>* event_queue);

int insert_process(vector<Process>* stat_info, Process* proc);

//int insert_queue(deque<Event>* event_queue, Event eve);

int put_event(deque<Event>* event_queue, Process* process, int old_state, vector<long>* rand_num, vector<long>::iterator* rand_ite, int cur_time, int* cur_end_time);

int create_info (MidInfo* info, int start_time, int process_num, int last_time, int prev_state, int next_state,int cb, int ib, int rem, int prio);

void print_info(MidInfo info);

void get_io_utiliz(vector<MidInfo>* info_vec, double* io_utilize);

void print_process(Process proc);

int get_next_event_time(deque<Event>* event_queue);

int print_sum(char sched_type, vector<Process>* stat_info, vector<MidInfo>* info_vec);

int print_mid_res(vector<MidInfo>* vec);

int simulation(ifstream* file, vector<long>* rand_num, vector<long>::iterator* rand_ite, deque<Event>* event_queue, vector<Process>* stat_info, vector<MidInfo>* info_vec);

int init_event_proc(ifstream* file, deque<Event>* event_queue, int max_prio, vector<long>* rand_num, vector<long>::iterator* rand_ite);

int get_num(char* sched_name);
