#include "sched.h"
using namespace std;

int lineNum = 0;

Scheduler* sched;  // that's the only object we use in global algo

Event* DES_layer::get_event(){
	if (event_queue.size() > 0 ){
		Event* tmp = &(event_queue.front());
		event_queue.pop_front();
		return tmp;
	}else
		return NULL;
}

int DES_layer::put_event(Process* process, int old_state, int state_prev_prev, vector<long>* rand_num, vector<long>::iterator* rand_ite, int cur_time, int* cur_end_time, bool is_preempt){
        Event event;
	event.process = process;
	event.old_state = old_state;
	int run_time = 0;
	if (is_preempt == true){
                        event.old_state = STATE_RUNNING;
                        event.new_state = STATE_READY;
                        event.transition = TRANS_TO_PREEMPT;
                        event.timestamp = cur_time;
	}
	else if (event.old_state == STATE_READY and state_prev_prev == STATE_CREATED){
					if (process->state_ts > *cur_end_time)
						event.timestamp = process->state_ts;
					else
						event.timestamp = *cur_end_time;
					event.new_state = STATE_RUNNING;
					event.transition = TRANS_TO_RUNNING;
	}
	else if (event.old_state == STATE_READY and state_prev_prev == STATE_BLOCK){
		if (process->state_ts > *cur_end_time)
						event.timestamp = process->state_ts;
					else
						event.timestamp = *cur_end_time;
					event.new_state = STATE_RUNNING;
					event.transition = TRANS_TO_RUNNING;
	}
	else if (event.old_state == STATE_READY and state_prev_prev == STATE_RUNNING){
					if (process->state_ts > *cur_end_time)
						event.timestamp = process->state_ts;
					else
						event.timestamp = *cur_end_time;
					event.new_state = STATE_RUNNING;
					event.transition = TRANS_TO_RUNNING;
	}
	else if (event.old_state == STATE_RUNNING){
		int quantum = sched->get_quantum();
		event.new_state = STATE_BLOCK;
		if (process->left_cb != 0)
			run_time = process->left_cb;
		else{
			try{
				run_time = my_random(process->cpu_max, rand_num, rand_ite);
			}catch (int){
				*rand_ite = rand_num->begin();
				run_time = my_random(process->cpu_max, rand_num, rand_ite);
			}
		}
		if (process->cpu_all_time < run_time)
			run_time = process->cpu_all_time;
		//if (process->left_cb != 0)
		//	process->left_cb = process->left_cb - run_time;
		if (run_time <= quantum){
			event.transition = TRANS_TO_BLOCK;
			event.timestamp = cur_time + run_time;
			if (process->left_cb != 0)
				process->left_cb = process->left_cb - run_time;
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
		try{
			run_time = my_random(process->io_max, rand_num, rand_ite);
		}catch (int){
			*rand_ite = rand_num->begin();
			run_time = my_random(process->cpu_max, rand_num, rand_ite);
		}
		event.timestamp = cur_time + run_time;
		(*cur_end_time) = cur_time;
	}
	insert_queue(&event_queue, event);
	return run_time;
}

int DES_layer::get_next_event_time(){
	if (&(event_queue.front()) != NULL)
		return (event_queue.front()).timestamp;
	else
		return -1;	
}

int DES_layer::remove_event(Process* cur_proc){
	deque<Event>::iterator ite = event_queue.begin();
	while(ite != event_queue.end()){
		if (ite->process != cur_proc)
			ite++;
		else
			break;
	}
	event_queue.erase(ite);
	return 0;
}

int init_event_proc(ifstream* file, int max_prio, vector<long>* rand_num, vector<long>::iterator* rand_ite, DES_layer* des){
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
		proc->state = STATE_READY;
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
		des->event_queue.push_back(event);					 

	}
	//create each process's static prio and prio
	deque<Event>::iterator ite = des->event_queue.begin();
	while(ite != des->event_queue.end()){
		try{
			((*ite).process)->static_prio = my_random(max_prio, rand_num, rand_ite);
		}catch (int){
			*rand_ite = rand_num->begin();
			((*ite).process)->static_prio = my_random(max_prio, rand_num, rand_ite);
		}
		((*ite).process)->prio = ((*ite).process)->static_prio - 1;
		ite++;
	}
	
	return 0;	
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

int my_random(int up_limit, vector<long>* rand_num, vector<long>::iterator* rand_ite){
	int rand_int = *(*rand_ite);
	if (up_limit == 0)
		up_limit = defLimit;
	 int rand_res = ((**rand_ite) % up_limit) + 1;
	 //printf("rand_res %d\n", rand_res);
	 if (*rand_ite != rand_num->end())
	 	(*rand_ite)++;
	 else 
		throw -1;
	 return rand_res;
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

bool Scheduler::does_preempt(Process* cur_proc, Process** new_proc, deque<Event>* event_queue, int cur_time){
	return false;
}

void Scheduler_FCFS::add_to_queue(Process* proc) {
        if (proc->prio == -1)
            proc->prio = proc->static_prio - 1;
	run_queue.push_back(proc);
}

Process* Scheduler_FCFS::get_from_queue() {
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

bool Scheduler_FCFS::does_preempt(Process* cur_proc, Process** new_proc, deque<Event>* event_queue, int cur_time){
	return false;
}

void Scheduler_LCFS::add_to_queue(Process* proc) {
        if (proc->prio == -1)
            proc->prio = proc->static_prio - 1;
        run_queue.push_back(proc);
}

Process* Scheduler_LCFS::get_from_queue() {
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

bool Scheduler_LCFS::does_preempt(Process* cur_proc, Process** new_proc, deque<Event>* event_queue, int cur_time){
	return false;
}

void Scheduler_SRTF::add_to_queue(Process* proc) {
        if (proc->prio == -1)
            proc->prio = proc->static_prio - 1;
        deque<Process*>::iterator ite = run_queue.begin();
        if(run_queue.size() == 0)
                run_queue.push_back(proc);
        else{
                while(ite != run_queue.end()){
                if (proc->cpu_all_time >= (*ite)->cpu_all_time)
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

Process* Scheduler_SRTF::get_from_queue() {
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

bool Scheduler_SRTF::does_preempt(Process* cur_proc, Process** new_proc, deque<Event>* event_queue, int cur_time){
	return false;
}

void Scheduler_RR::add_to_queue(Process* proc) {
        proc->prio = proc->static_prio-1;
        run_queue.push_back(proc);
}

Process* Scheduler_RR::get_from_queue() {
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

bool Scheduler_RR::does_preempt(Process* cur_proc, Process** new_proc, deque<Event>* event_queue, int cur_time){
	return false;
}

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

Process* Scheduler_PRIO::get_from_queue() {
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

bool Scheduler_PRIO::does_preempt(Process* cur_proc, Process** new_proc, deque<Event>* event_queue, int cur_time){
	return false;
}

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

Process* Scheduler_EPRIO::get_from_queue() {
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

int Scheduler_EPRIO::get_quantum() {
        return quantum;
}

void Scheduler_EPRIO::set_quantum(int num) {
 quantum = num;
}

bool Scheduler_EPRIO::does_preempt(Process* cur_proc, Process** new_proc, deque<Event>* event_queue, int cur_time){
	bool is_preempt = false;
	Process* tmp;
       for (int i=max_prio-1; i >=0; i--){
                        if ((run_queue_list[i]).size() > 0){
                                if (cur_proc->prio < i){
                                	is_preempt = true;
					tmp = run_queue_list[i].front();
                                        break;
                                }
                        }
                }
	deque<Event>::iterator ite = event_queue->begin();
	while(ite != event_queue->end()){
		if ((*ite).process == cur_proc){
			if ((*ite).timestamp == cur_time){
				is_preempt = false;
				break;
			}
		}
		ite++;
	}
	if (is_preempt == true)
		*new_proc = tmp;
	return is_preempt; 	
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


int print_sum(char sched_type, vector<Process>* stat_info, vector<MidInfo>* info_vec){
	switch (sched_type) {
		case FCFS:
			printf("FCFS\n");
			break;
		case LCFS: 
			printf("LCFS\n");
			break;
		case SRTF: 
			printf("SRTF\n");
			break;
		case RR: {
			int quantum = sched->get_quantum();
			printf("RR %d\n", quantum);
			break;
		}
		case PRIO: {
			int quantum = sched->get_quantum();
			printf("PRIO %d\n", quantum);
			break;
		}
		case EPRIO: {
			int quantum = sched->get_quantum();
			printf("PREPRIO %d\n", quantum);
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
	cpu_utiliz = cpu_utiliz_time/total_time*100.0;
	get_io_utiliz(info_vec, &io_utiliz_time);
	io_utiliz = io_utiliz_time/total_time*100.0;
	printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", total_time, cpu_utiliz, io_utiliz, avg_run_time, avg_wait_time, through_put);		
}

int print_mid_res(vector<MidInfo>* vec){
	vector<MidInfo>::iterator ite = vec->begin();
	while (ite != vec->end()){
		print_info(*ite);
		ite++;
	}
	return 0;
}

int simulation(ifstream* file, vector<long>* rand_num, vector<long>::iterator* rand_ite, vector<Process>* stat_info, vector<MidInfo>* info_vec, DES_layer* des){
	Event* evt;
	bool CALL_SCHEDULER = true;
	Process* cur_proc = NULL;
	int cur_end_time = 0; 
	while (evt = des->get_event()){
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
				if (proc->state_prev == STATE_CREATED){
					//come from CREATED
				         //print info for CREATED->READY
				        //proc->state = STATE_READY;
				        create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, 0, 0, proc->cpu_all_time, proc->prio);	
					info_vec->push_back(info);
					//print_info(info);
					
					proc->state_prev_prev = STATE_CREATED;
					proc->state_prev = STATE_READY;
					proc->state = STATE_RUNNING;
				} 
				else {
					//come from BLOCKED
					//print info for BLOCK->READY 
					create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, 0, 0, proc->cpu_all_time, proc->static_prio);
					info_vec->push_back(info);
					//print_info(info);
					
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
				proc->state_ts = cur_time;	
				proc->cpu_all_time = proc->cpu_all_time - timeInPrev;	
				proc->cpu_utiliz_time -= proc->left_cb;
				//print info for RUNNING->READY	
				create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, proc->left_cb, 0, proc->cpu_all_time, proc->prio);	
				info_vec->push_back(info);
				//print_info(info);
				
				proc->state_prev_prev = STATE_RUNNING;
				proc->state_prev = STATE_READY;
				proc->state = STATE_RUNNING;
				proc->prio = proc->prio - 1;
				sched->add_to_queue(proc);	
				cur_proc = NULL;		
				break;
				}
			case TRANS_TO_RUNNING:
				{
				//comes from READY
				//create event for next step, it is either preempt or RUNNING->BLOCK
				proc->state_ts = cur_time;
				int cb = des->put_event(proc, STATE_RUNNING, STATE_READY, rand_num, rand_ite, cur_time, &cur_end_time, false);
				
				//print READY->RUNNING info
				create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, cb, 0, proc->cpu_all_time, proc->prio);
				info_vec->push_back(info);
				//print_info(info);
				//add stat info 
				proc->cpu_utiliz_time += cb;
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
						int ib = des->put_event(proc, STATE_BLOCK, STATE_RUNNING, rand_num, rand_ite, cur_time, &cur_end_time, false);
						//print RUNNING->BLOCK info
						create_info(&info, proc->state_ts, proc->num, timeInPrev, proc->state_prev, proc->state, 0, ib, proc->cpu_all_time, proc->static_prio);
						info_vec->push_back(info);
						//print_info(info);
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
						//print_info(info);
					        //set statistical info
					        proc->finish_time = cur_time;
						proc->run_time = proc->finish_time - proc->start_time;
						insert_process(stat_info, proc);	
					}
					proc->prio = proc->static_prio - 1;
					cur_proc = NULL;
				break;
		}
		if(CALL_SCHEDULER){
			if (des->get_next_event_time() == cur_time)
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
                 			des->put_event(cur_proc, STATE_READY, STATE_CREATED, rand_num, rand_ite, cur_time, &cur_end_time, false);
				}else if (cur_proc->state_prev == STATE_READY and cur_proc->state_prev_prev == STATE_BLOCK){
                 			des->put_event(cur_proc, STATE_READY, STATE_BLOCK, rand_num, rand_ite, cur_time, &cur_end_time, false);
				}else if (cur_proc->state_prev == STATE_READY and cur_proc->state_prev_prev == STATE_RUNNING){
                 			des->put_event(cur_proc, STATE_READY, STATE_RUNNING, rand_num, rand_ite, cur_time, &cur_end_time, false);
				}
			}
			else{
				Process* new_proc;
				bool is_preempt = sched->does_preempt(cur_proc, &new_proc, &(des->event_queue), cur_time);
				if (is_preempt == true){
					des->remove_event(cur_proc);
					des->put_event(cur_proc, STATE_RUNNING, STATE_READY, rand_num, rand_ite, cur_time, &cur_end_time, is_preempt);	
				        cur_proc->left_cb = cur_end_time + cur_proc->left_cb - cur_time;
                        		cur_proc->state_prev_prev = STATE_READY;
                        		cur_proc->state_prev = STATE_RUNNING;
                        		cur_proc->state = STATE_READY;
                        		cur_end_time = cur_time;	
					cur_proc = new_proc;
				}
			}
		}
	}
}

int get_num(char* sched_name){
	int len = strlen(sched_name);   
	char num [len+1];
	int cnt = 0;
	int res = 0;
	for (int i=0; i < len; i++){
		if (isdigit(sched_name[i]))
			num[cnt++] = sched_name[i];
		else{
			break;
		}
	}
	num[cnt] = '\0';
	res = atoi(num);
	return res;
}

int main (int argc, char* argv[])
{
	int c;
	int sched_type = 0;
	char* sched_name = NULL;
	int quantum = 0;
	int is_print_mid = 0;
	int max_prio = 0;

	while ((c = getopt(argc, argv, "vs:")) != -1 )
	{
		switch(c) {
			case 's':{
					 sched_name = optarg;	
					 if (sched_name[0] == 'F'){
						 sched_type = FCFS;
						 quantum = maxQuan;
						 max_prio = maxPrioDef;
					 }
					 else if(sched_name[0] == 'L'){
						 sched_type = LCFS;
						 quantum = maxQuan;
						 max_prio = maxPrioDef;	
					 }
					 else if(sched_name[0] == 'S'){
						 sched_type = SRTF;
						 quantum = maxQuan;
						 max_prio = maxPrioDef;	 
					 }
					 else if(sched_name[0] == 'R'){
						 sched_type = RR;	
						 if(!isdigit(sched_name[1])){
							 printf("Invalid scheduler param <%s>\n", sched_name);
							 return -1;
						 }else{
							 quantum = get_num(&(sched_name[1]));
							 if (quantum == 0){
								 printf("Invalid scheduler param <%s>\n", sched_name);
								 return -1;
							 }
							 max_prio = maxPrioDef;
						 }	
					 }
					 else if(sched_name[0] == 'P'){
						 sched_type = PRIO;
						 int len = strlen(sched_name);
						 char tmp_sched_name [len];
						 for (int i=0; i < len; i++)
							 tmp_sched_name[i] = sched_name[i];	 
						 const char* delim = ":";
						 char* quantum_str = strtok(&(tmp_sched_name[1]), delim);
						 char* prio_str = strtok(NULL, delim);
						 if(!isdigit(quantum_str[0])){
							 printf("Invalid scheduler param <%s>\n", sched_name);
							 return -1;
						 }else{
							 quantum = get_num(quantum_str);
							 if (quantum == 0){
								 printf("Invalid scheduler param <%s>\n", sched_name);
								 return -1;
							 }
							 if (prio_str == NULL){
								 max_prio = maxPrioDef;
							 }
							 else{
								 if (!isdigit(prio_str[0])){
									 printf("Invalid scheduler param <%s>\n", sched_name);
									 return -1;
								 }
								 else{
									 max_prio = get_num(prio_str);
									 if (max_prio == 0){
										 printf("Invalid scheduler param <%s>\n", sched_name);
										 return -1;
									 }
								 }
							 }
						 }
					 }
					 else if(sched_name[0] == 'E'){
						 sched_type = EPRIO;
						 int len = strlen(sched_name);
						 char tmp_sched_name [len];
						 for (int i=0; i < len; i++)
							 tmp_sched_name[i] = sched_name[i];	 
						 const char* delim = ":";
						 char* quantum_str = strtok(&(tmp_sched_name[1]), delim);
						 char* prio_str = strtok(NULL, delim);
						 if(!isdigit(quantum_str[0])){
							 printf("Invalid scheduler param <%s>\n", sched_name);
							 return -1;
						 }else{
							 quantum = get_num(quantum_str);
							 if (quantum == 0){
								 printf("Invalid scheduler param <%s>\n", sched_name);
								 return -1;
							 }
							 if (prio_str == NULL){
								 max_prio = maxPrioDef;
							 }
							 else{
								 if (!isdigit(prio_str[0])){
									 printf("Invalid scheduler param <%s>\n", sched_name);
									 return -1;
								 }
								 else{
									 max_prio = get_num(prio_str);
									 if (max_prio == 0){
										 printf("Invalid scheduler param <%s>\n", sched_name);
										 return -1;
									 }
								 }
							 }
						 }
					 }
					 else{	
						 printf("Unknown Scheduler spec: -v {FLSRPE}\n");			
						 return -1;
					 }
					 break;
				 }
			case 'v':
				 is_print_mid = 1;
				 break;
			case '?':
				 printf("Usage: ./sched [-v] [-t] [-e] [-p] [-i] [-s sched] inputfile randomfile\n");
				 printf("   -v enables verbose\n");
				 printf("   -t enables scheduler details\n");
				 printf("   -e enables event tracing\n");
				 printf("   -p enables E scheduler preempton tracing\n");
				 printf("   -i single steps event by event\n");
				 return -1;
		}
	}

	if (sched_type == 0){
		sched_type = FCFS;
		quantum = maxQuan;
		max_prio = maxPrioDef;
	}

			switch (sched_type) {
				case FCFS:{
						  Scheduler_FCFS* fcfs_scheduler = new Scheduler_FCFS();  
						  fcfs_scheduler->set_quantum(quantum);
						  sched = fcfs_scheduler;
						  break;
					  }
				case LCFS:{
						  Scheduler_LCFS* lcfs_scheduler = new Scheduler_LCFS();  
						  lcfs_scheduler->set_quantum(quantum);
						  sched = lcfs_scheduler;
						  break;
					  }
				case SRTF:{
						  Scheduler_SRTF* srtf_scheduler = new Scheduler_SRTF();  
						  srtf_scheduler->set_quantum(quantum);
						  sched = srtf_scheduler;
						  break;
					  }
				case RR:{
						Scheduler_RR* rr_scheduler = new Scheduler_RR();  
						rr_scheduler->set_quantum(quantum);
						sched = rr_scheduler;
						break;
					}
				case PRIO:{
						  Scheduler_PRIO* prio_scheduler = new Scheduler_PRIO(max_prio);  
						  prio_scheduler->set_quantum(quantum);
						  sched = prio_scheduler;
						  break;
					  }
				case EPRIO:{
						   Scheduler_EPRIO* eprio_scheduler = new Scheduler_EPRIO(max_prio);  
						   eprio_scheduler->set_quantum(quantum);
						   sched = eprio_scheduler;
						   break;
					   }
			}

	//open input file
	if (argc < optind+2){
		printf("input arguments not enough\n");
		return -1;
	}
	char* file_path = argv[optind++];
	ifstream file(file_path);
	if (!file.is_open()){
		printf("no open input file\n");
		return -1;	
	}

	//open rand file 
	char* rand_file_path = argv[optind];
	ifstream rand_file(rand_file_path);
	if (!rand_file.is_open()){
		printf("no open rand file\n");
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
	class DES_layer des;
	init_event_proc(&file, max_prio, &rand_num, &rand_ite, &des);

	vector<Process> stat_proc_info;
	vector<MidInfo> info_vec;
	simulation(&file, &rand_num, &rand_ite, &stat_proc_info, &info_vec, &des);

	if (is_print_mid == 1){
		print_mid_res(&info_vec);
	}

	//print summation
	print_sum(sched_type, &stat_proc_info, &info_vec);
	return 0; 	


}

