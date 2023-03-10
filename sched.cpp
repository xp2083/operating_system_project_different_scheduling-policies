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
#define DEBUG 1
#define defLimit 4

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
int rand_cnt = 2; 

class Scheduler_FCFS: public Scheduler {
	public:
	~Scheduler_FCFS() {};
	void add_to_queue (Process* proc);
        Process* get_from_queue ();
	int get_quantum ();
	void set_quantum (int num);
	int get_queue_size ();
};

void Scheduler_FCFS::add_to_queue(Process* proc) {
	run_queue.push_back(proc);
	//printf("in add_to_queue size of sched queue %d\n", sched->get_queue_size());
	//printf("%s Q=%d\n", __FUNCTION__, obj->run_queue);
}

Process* Scheduler_FCFS::get_from_queue() {
	//printf("%s\n", __FUNCTION__);
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
        Process* get_from_queue ();
	int get_quantum ();
	void set_quantum (int num);
	int get_queue_size ();
};

void Scheduler_LCFS::add_to_queue(Process* proc) {
	run_queue.push_back(proc);
	//printf("in add_to_queue size of sched queue %d\n", sched->get_queue_size());
	//printf("%s Q=%d\n", __FUNCTION__, obj->run_queue);
}

Process* Scheduler_LCFS::get_from_queue() {
	//printf("%s\n", __FUNCTION__);
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
        Process* get_from_queue ();
	int get_quantum ();
	void set_quantum (int num);
	int get_queue_size ();
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
	//printf("in add_to_queue size of sched queue %d\n", sched->get_queue_size());
	//printf("%s Q=%d\n", __FUNCTION__, obj->run_queue);
	}
}

Process* Scheduler_SRTF::get_from_queue() {
	//printf("%s\n", __FUNCTION__);
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
	~Scheduler_RR() {};
	void add_to_queue (Process* proc);
        Process* get_from_queue ();
	int get_quantum ();
	void set_quantum (int num);
	int get_queue_size ();
};

void Scheduler_RR::add_to_queue(Process* proc) {
	run_queue.push_back(proc);
}

Process* Scheduler_RR::get_from_queue() {
	//printf("%s\n", __FUNCTION__);
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
	~Scheduler_PRIO() {};
	void add_to_queue (Process* proc);
        Process* get_from_queue ();
	int get_quantum ();
	void set_quantum (int num);
	int get_queue_size ();
};

void Scheduler_PRIO::add_to_queue(Process* proc) {
	deque<Process*>::iterator ite = run_queue.begin();
	if(run_queue.size() == 0)
		run_queue.push_back(proc);
	else{
		while(ite != run_queue.end()){
		if (proc->prio <= (*ite)->prio)
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

Process* Scheduler_PRIO::get_from_queue() {
	//printf("%s\n", __FUNCTION__);
	if (run_queue.size() != 0){
		Process* tmp = run_queue.front();
		run_queue.pop_front();
		return tmp;
	}
	return NULL;
}

int Scheduler_PRIO::get_quantum() {
        return quantum;
}

void Scheduler_PRIO::set_quantum(int num) {
        quantum = num;
}
///////////////////////////////////////

int my_random(int up_limit, vector<long>* rand_num, vector<long>::iterator* rand_ite){
	if (*rand_ite == rand_num->end()){
		*rand_ite = rand_num->begin();
	} 	 
	 if (up_limit == 0)
		up_limit = defLimit;	
	 int rand_res = ((**rand_ite) % up_limit) + 1;
	 //printf("rand_num %lu\n", rand_num[rand_cnt]);
	 //printf("rand_res %d\n", rand_res);
	 (*rand_ite)++;
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

int insert_process(vector<Process>* stat_info, Process* proc){
	vector<Process>::iterator ite = stat_info->begin();
	if (stat_info->size() == 0)
		stat_info->push_back(*proc);
	else{
		while(ite != stat_info->end()){
			if (proc->num > ite->num)
				ite++;
			else
				break;
		}
		if (ite != stat_info->end())
			stat_info->insert(ite, *proc);
		else
			stat_info->push_back(*proc);
	}
}

int insert_queue(deque<Event>* event_queue, Event eve){
	deque<Event>::iterator ite = event_queue->begin();
	if (event_queue->size() == 0){
		event_queue->push_back(eve);
	}
	else{
		while (ite != event_queue->end()){
			if (eve.timestamp >= ite->timestamp) 
				ite++;
			else 
				break;
		}
		if (ite != event_queue->end())
			event_queue->insert(ite, eve);
		else
			event_queue->push_back(eve);
	}
	return 0;
}

int put_event(deque<Event>* event_queue, Process* process, int old_state, vector<long>* rand_num, vector<long>::iterator* rand_ite, int cur_time, int* cur_end_time){
        Event event;
	event.process = process;
	event.old_state = old_state;
	int run_time = 0;
	if (event.old_state == STATE_RUNNING){
		int quantum = sched->get_quantum();
		event.new_state = STATE_BLOCK;
		if (process->left_cb != 0)
			run_time = process->left_cb;
		else
			run_time = my_random(process->cpu_max, rand_num, rand_ite);
		if (process->cpu_all_time < run_time)
			run_time = process->cpu_all_time;
		if (process->left_cb != 0)
			process->left_cb = process->left_cb - run_time;
		if (run_time <= quantum){
			event.transition = TRANS_TO_BLOCK;
			event.timestamp = cur_time + run_time;
		}
		else{
			//when preempt happens
			event.transition = TRANS_TO_PREEMPT;
			event.timestamp = cur_time + quantum;
			process->left_cb = run_time - quantum; 
		}
		//if process is still running in cpu
		(*cur_end_time) = event.timestamp;		
	}
	else if (event.old_state == STATE_BLOCK){
		event.new_state	= STATE_READY;
		event.transition = TRANS_TO_READY;
		run_time = my_random(process->io_max, rand_num, rand_ite);
		event.timestamp = cur_time + run_time;
		(*cur_end_time) = cur_time;
	}
	insert_queue(event_queue, event);
	return run_time;
}

int create_info (MidInfo* info, int start_time, int process_num, int last_time, int prev_state, int next_state,int cb, int ib, int rem, int prio) {
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


void print_info(MidInfo info){
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
	else if (info.prev_state == STATE_RUNNING and info.next_state == STATE_BLOCK)
		printf("  ib=%d rem=%d\n", info.ib, info.rem);
	else if (info.prev_state == STATE_RUNNING and info.next_state == STATE_READY)
		printf("  cb=%d rem=%d prio=%d\n", info.cb, info.rem, info.prio);
	
	}	
}

void get_io_utiliz(vector<MidInfo>* info_vec, double* io_utilize){
	int last_ib_start = 0;
	int last_ib_end = 0; 
	vector<MidInfo>::iterator ite = info_vec->begin();
	while (ite != info_vec->end()){
		MidInfo info = *ite;
		if (info.prev_state == STATE_RUNNING){
			//if the next ib step is in all the range of the last ib step
			//do nothing
			if (info.s_time + info.ib > last_ib_end){
			  //if the next ib step's start is within last ib start and last ib end
			  //set a new start
			  int tmp_start = 0;
			  if (info.s_time < last_ib_end)
			  	tmp_start = last_ib_end;
			  else
				tmp_start = info.s_time;		
			
			*io_utilize += info.s_time + info.ib - tmp_start;
			last_ib_start = info.s_time;
			last_ib_end = info.s_time + info.ib;
			}
		}
		ite++;
	}
	int t = 0;
}

void print_process(Process proc){
	printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", proc.num, proc.start_time, proc.total_cpu, proc.cpu_max, proc.io_max, proc.static_prio, proc.finish_time, proc.run_time, proc.io_time, proc.cpu_wait_time);
} 

int get_next_event_time(deque<Event>* event_queue){
	if (&(event_queue->front()) != NULL)
		return (event_queue->front()).timestamp;
	else
		return -1;	

}

int print_sum(int sched_type, vector<Process>* stat_info, vector<MidInfo>* info_vec){
	switch (sched_type) {
		case 1:
			printf("FCFS\n");
			break;
		case 2: 
			printf("LCFS\n");
			break;
		case 3: 
			printf("SRTF\n");
			break;
		case 4: {
			int quantum = sched->get_quantum();
			printf("RR %d\n", quantum);
			break;
		}
		case 5: {
			int quantum = sched->get_quantum();
			printf("PRIO %d\n", quantum);
			break;
		}
		default:
			exit(1);
	}
	int total_time = 0;
	double cpu_utiliz_time = 0.0;
	double cpu_utiliz = 0.0;
	double io_utiliz_time = 0.0;
	double io_utiliz = 0.0;
	double avg_run_time = 0.0;
	double avg_wait_time = 0.0;
	double through_put = 0.0;
	int cnt = 0;
	vector<Process>::iterator ite = stat_info->begin();
	while (ite != stat_info->end()){
		cnt += 1;
		Process proc = *ite;
		print_process(proc); 
		if (proc.finish_time > total_time)
			total_time = proc.finish_time;
		avg_run_time += proc.run_time;
		avg_wait_time += proc.cpu_wait_time;
		cpu_utiliz_time += proc.cpu_utiliz_time;
		ite++;
		}
	avg_run_time /= cnt;
	avg_wait_time /= cnt;
	through_put = 100.00/total_time*cnt;
	cpu_utiliz = cpu_utiliz_time/total_time*100;
	get_io_utiliz(info_vec, &io_utiliz_time);
	io_utiliz = io_utiliz_time/total_time*100;
	printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", total_time, cpu_utiliz, io_utiliz, avg_run_time, avg_wait_time, through_put);		
}

int simulation(ifstream* file, vector<long>* rand_num, vector<long>::iterator* rand_ite, deque<Event>* event_queue, vector<Process>* stat_info, vector<MidInfo>* info_vec){
	Event* evt;
	bool CALL_SCHEDULER = true;
	Process* cur_proc = NULL;
	int cur_end_time = 0; 
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
				proc->state_ts = cur_time;
				if (proc->state == STATE_CREATED){
					//come from CREATED
				         //print info for CREATED->READY
				        proc->state = STATE_READY;
				        create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, 0, 0, proc->cpu_all_time, proc->prio);	
					info_vec->push_back(info);
					#ifdef DEBUG
					print_info(info);
					#endif
					
					proc->state_prev_prev = STATE_CREATED;
					proc->state_prev = STATE_READY;
					proc->state = STATE_RUNNING;
				} 
				else {
					//come from BLOCKED
					//print info for BLOCK->READY 
					create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, 0, 0, proc->cpu_all_time, proc->static_prio);
					info_vec->push_back(info);
					#ifdef DEBUG
					print_info(info);
					#endif
					
					proc->state_prev_prev = STATE_BLOCK;
					proc->state_prev = STATE_READY;
					proc->state = STATE_RUNNING;
				}
				break;

			case TRANS_TO_PREEMPT:
				{
				//must come from RUNNING 
				//add to run queue, no event is generated
				CALL_SCHEDULER = true;
				//sched->add_to_queue(proc);	
				proc->state_ts = cur_time;	
				proc->cpu_all_time = proc->cpu_all_time - timeInPrev;	
				//print info for RUNNING->READY	
				create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, proc->left_cb, 0, proc->cpu_all_time, proc->prio);	
				info_vec->push_back(info);
				#ifdef DEBUG
				print_info(info);
				#endif
				
				proc->state_prev_prev = STATE_RUNNING;
				proc->state_prev = STATE_READY;
				proc->state = STATE_RUNNING;
				proc->prio = proc->prio-1;
				if (proc->prio == -1)
					proc->prio = proc->static_prio - 1;
				sched->add_to_queue(proc);	
				cur_proc = NULL;		
				break;
				}
			case TRANS_TO_RUNNING:
				{
				//comes from READY
				//create event for next step, it is either preempt or RUNNING->BLOCK
				proc->state_ts = cur_time;
				int cb = put_event(event_queue, proc, STATE_RUNNING, rand_num, rand_ite, cur_time, &cur_end_time);
				
				//print READY->RUNNING info
				create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, cb, 0, proc->cpu_all_time, proc->prio);
				info_vec->push_back(info);
				#ifdef DEBUG
				print_info(info);
				#endif
				//add stat info 
				proc->cpu_utiliz_time += cb - proc->left_cb;
				proc->cpu_wait_time += timeInPrev;
	
				//for print info
				if (proc->left_cb == 0)
					proc->state = STATE_BLOCK;
				else
					proc->state = STATE_READY;
				proc->state_prev = STATE_RUNNING;
				proc->state_prev_prev = STATE_READY;
				break;
			}

			case TRANS_TO_BLOCK:
					proc->state_ts = cur_time;
					proc->cpu_all_time = proc->cpu_all_time - timeInPrev;
					CALL_SCHEDULER = true;
					if (proc->cpu_all_time > 0){
						//create an event for BLOCK->READY
						int ib = put_event(event_queue, proc, STATE_BLOCK, rand_num, rand_ite, cur_time, &cur_end_time);
						//print RUNNING->BLOCK info
						create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, 0, ib, proc->cpu_all_time, proc->static_prio);
						info_vec->push_back(info);
						#ifdef DEBUG
						print_info(info);
						#endif
						//add stat info
						proc->io_time += ib;

						//set process state for next session
						proc->state = STATE_READY;
						proc->state_prev = STATE_BLOCK;
						proc->state_prev_prev = STATE_RUNNING;
					}
					else{
						create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, timeInPrev, 0, proc->cpu_all_time, proc->prio);
						info_vec->push_back(info);
					        #ifdef DEBUG
						print_info(info);
						#endif
					        //set statistical info
					        proc->finish_time = cur_time;
						proc->run_time = proc->finish_time - proc->start_time;
						insert_process(stat_info, proc);	
					}
					proc->prio = proc->static_prio-1;
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
				//create event for READY->RUNNING
				if (cur_proc->state_prev == STATE_READY and cur_proc->state_prev_prev == STATE_CREATED){
					Event event;
					if (cur_proc->state_ts > cur_end_time)
						event.timestamp = cur_proc->state_ts;
					else
						event.timestamp = cur_end_time;
					event.process = cur_proc;
					event.old_state = STATE_READY;
					event.new_state = STATE_RUNNING;
					event.transition = TRANS_TO_RUNNING;
					insert_queue(event_queue, event);
				}else if (cur_proc->state_prev == STATE_READY and cur_proc->state_prev_prev == STATE_BLOCK){
					Event event;
					if (cur_proc->state_ts > cur_end_time)
						event.timestamp = cur_proc->state_ts;
					else
						event.timestamp = cur_end_time;
					event.process = cur_proc;
					event.old_state = STATE_READY;
					event.new_state = STATE_RUNNING;
					event.transition = TRANS_TO_RUNNING;
					insert_queue(event_queue, event);
				}else if (cur_proc->state_prev == STATE_READY and cur_proc->state_prev_prev == STATE_RUNNING){
					Event event;
					if (cur_proc->state_ts > cur_end_time)
						event.timestamp = cur_proc->state_ts;
					else
						event.timestamp = cur_end_time;
					event.process = cur_proc;
					event.old_state = STATE_READY;
					event.new_state = STATE_RUNNING;
					event.transition = TRANS_TO_RUNNING;
					insert_queue(event_queue, event);
				}
			}
		}
	}
}

int init_event_proc(ifstream* file, deque<Event>* event_queue, int max_prio, vector<long>* rand_num, vector<long>::iterator* rand_ite){
	int proc_cnt = 0;
	while (file->peek() != EOF){
		proc_cnt++;
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
		proc->state = STATE_CREATED;
		proc->state_prev = STATE_CREATED;
		proc->state_prev_prev = STATE_CREATED;
		proc->static_prio = 0;
		proc->prio = 0;
		proc->num = lineNum;
		proc->left_cb = 0;
		proc->start_time = proc->state_ts;
		proc->total_cpu = proc->cpu_all_time;
		proc->finish_time = 0;
		proc->run_time = 0;
		proc->io_time = 0;
		proc->cpu_wait_time = 0;
		proc->cpu_utiliz_time = 0;
		proc->last_ib_start = 0;
		proc->last_ib_end = 0;
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
	//create each process's static prio and prio
	deque<Event>::iterator ite = event_queue->begin();
	while(ite != event_queue->end()){
		((*ite).process)->static_prio = my_random(max_prio, rand_num, rand_ite);
		((*ite).process)->prio = ((*ite).process)->static_prio - 1;
		ite++;
	}
	
	return 0;	
}

int main (int argc, char* argv[])
{
	//create scheduler
	int c;
	int sched_type = 5;
	int quantum = 10;
        /*
	while ((c = getopt(argc,argv,"s:")) != -1 )
        {
                switch(c) {
                case 's':
                        schedtype = atoi(optarg);
                        break;
                }
        }*/

	int max_prio = 5;

        switch (sched_type) {
        case 1:
		{
	    Scheduler_FCFS* fcfs_scheduler = new Scheduler_FCFS();  
	    fcfs_scheduler->set_quantum(1000000);
            sched = fcfs_scheduler;
            break;
		}
        case 2:{
	    Scheduler_LCFS* lcfs_scheduler = new Scheduler_LCFS();  
	    lcfs_scheduler->set_quantum(1000000);
            sched = lcfs_scheduler;
            break;
		}
        case 3:{
	    Scheduler_SRTF* srtf_scheduler = new Scheduler_SRTF();  
	    srtf_scheduler->set_quantum(1000000);
            sched = srtf_scheduler;
            break;
		}
        case 4:{
	    Scheduler_RR* rr_scheduler = new Scheduler_RR();  
	    rr_scheduler->set_quantum(quantum);
            sched = rr_scheduler;
            break;
		}
        case 5:{
	    Scheduler_PRIO* prio_scheduler = new Scheduler_PRIO();  
	    prio_scheduler->set_quantum(quantum);
            sched = prio_scheduler;
            break;
		}
	
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

	char tmp [16] = {0};
	rand_file.getline(tmp, sizeof(tmp));
	int randSize =  atoi(tmp);
	
	vector<long> rand_num;
	int cnt = 0;
	while (rand_file.peek() != EOF){		
		char tmp [16] = {0};
        	rand_file.getline(tmp, sizeof(tmp));	
        	rand_num.push_back(atol(tmp));
		cnt++;
	}
	
        vector<long>::iterator rand_ite = rand_num.begin();
        //initialize all process and their created->ready event 
 	deque<Event> event_queue;       
	init_event_proc(&file, &event_queue, max_prio, &rand_num, &rand_ite);

	vector<Process> stat_proc_info;
	vector<MidInfo> info_vec;
	simulation(&file, &rand_num, &rand_ite, &event_queue, &stat_proc_info, &info_vec);

	//print summation
	print_sum(sched_type, &stat_proc_info, &info_vec);
	return 0; 	

	
}

