#include "sched.h"
using namespace std;

int lineNum = 0;

Scheduler* sched;  // that's the only object we use in global algo

int my_random(int up_limit, vector<long>* rand_num, vector<long>::iterator* rand_ite){
	int rand_int = *(*rand_ite);
	if (*rand_ite == rand_num->end()){
		*rand_ite = rand_num->begin();
	} 	 
	 if (up_limit == 0)
		up_limit = defLimit;	
	 int rand_res = ((**rand_ite) % up_limit) + 1;
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

int print_sum(char sched_type, vector<Process>* stat_info, vector<MidInfo>* info_vec){
	switch (sched_type) {
		case 'F':
			printf("FCFS\n");
			break;
		case 'L': 
			printf("LCFS\n");
			break;
		case 'S': 
			printf("SRTF\n");
			break;
		case 'R': {
			int quantum = sched->get_quantum();
			printf("RR %d\n", quantum);
			break;
		}
		case 'P': {
			int quantum = sched->get_quantum();
			printf("PRIO %d\n", quantum);
			break;
		}
		case 'E': {
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
	cpu_utiliz = cpu_utiliz_time/total_time*100;
	get_io_utiliz(info_vec, &io_utiliz_time);
	io_utiliz = io_utiliz_time/total_time*100;
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
				//sched->add_to_queue(proc);	
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
				int cb = put_event(event_queue, proc, STATE_RUNNING, rand_num, rand_ite, cur_time, &cur_end_time);
				
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
						int ib = put_event(event_queue, proc, STATE_BLOCK, rand_num, rand_ite, cur_time, &cur_end_time);
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
			if (get_next_event_time(event_queue) == cur_time)
			//process next event from event queue
				continue;
			CALL_SCHEDULER = false;
			if (cur_proc == NULL){
				cur_proc = sched->get_from_queue(cur_proc, event_queue, cur_time, &cur_end_time);
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
			else{
				cur_proc = sched->get_from_queue(cur_proc, event_queue, cur_time, &cur_end_time);
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
	char sched_type;
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
						 sched_type = 'F';
						 quantum = maxQuan;
						 max_prio = maxPrioDef;
					 }
					 else if(sched_name[0] == 'L'){
						 sched_type = 'L';
						 quantum = maxQuan;
						 max_prio = maxPrioDef;	
					 }
					 else if(sched_name[0] == 'S'){
						 sched_type = 'S';
						 quantum = maxQuan;
						 max_prio = maxPrioDef;	 
					 }
					 else if(sched_name[0] == 'R'){
						 sched_type = 'R';	
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
						 sched_type = 'P';
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
						 sched_type = 'E';
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

	//printf("scheduler %c\n", sched_type);
	//printf("quantum %d\n", quantum);
	//printf("max_prio %d\n", max_prio);

        switch (sched_type) {
        case 'F':{
	    Scheduler_FCFS* fcfs_scheduler = new Scheduler_FCFS();  
	    fcfs_scheduler->set_quantum(quantum);
            sched = fcfs_scheduler;
            break;
		}
        case 'L':{
	    Scheduler_LCFS* lcfs_scheduler = new Scheduler_LCFS();  
	    lcfs_scheduler->set_quantum(quantum);
            sched = lcfs_scheduler;
            break;
		}
        case 'S':{
	    Scheduler_SRTF* srtf_scheduler = new Scheduler_SRTF();  
	    srtf_scheduler->set_quantum(quantum);
            sched = srtf_scheduler;
            break;
		}
        case 'R':{
	    Scheduler_RR* rr_scheduler = new Scheduler_RR();  
	    rr_scheduler->set_quantum(quantum);
            sched = rr_scheduler;
            break;
		}
        case 'P':{
	    Scheduler_PRIO* prio_scheduler = new Scheduler_PRIO(max_prio);  
	    prio_scheduler->set_quantum(quantum);
            sched = prio_scheduler;
            break;
		}
        case 'E':{
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
 	deque<Event> event_queue;       
	init_event_proc(&file, &event_queue, max_prio, &rand_num, &rand_ite);

	vector<Process> stat_proc_info;
	vector<MidInfo> info_vec;
	simulation(&file, &rand_num, &rand_ite, &event_queue, &stat_proc_info, &info_vec);

	if (is_print_mid == 1){
		print_mid_res(&info_vec);
	}

	//print summation
	print_sum(sched_type, &stat_proc_info, &info_vec);
	return 0; 	

	
}

