
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
#include <exception>
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

class DES_layer{
	public:
		deque<Event> event_queue;
		Event* get_event();
		int put_event(Process* process, int old_state, vector<long>* rand_num, vector<long>::iterator* rand_ite, int cur_time, int* cur_end_time);
		int get_next_event_time();				
};

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

class Scheduler_FCFS: public Scheduler {
	public:
		~Scheduler_FCFS() {};
		void add_to_queue (Process* proc);
		Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
		int get_quantum ();
		void set_quantum (int num);
};

class Scheduler_LCFS: public Scheduler {
	public:
		~Scheduler_LCFS() {};
		void add_to_queue (Process* proc);
		Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
		int get_quantum ();
		void set_quantum (int num);
};

class Scheduler_SRTF: public Scheduler {
	public:
		~Scheduler_SRTF() {};
		void add_to_queue (Process* proc);
		Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
		int get_quantum ();
		void set_quantum (int num);
};

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

class Scheduler_PRIO: public Scheduler {
	public:
		Scheduler_PRIO(int prio);
		~Scheduler_PRIO() {};
		void add_to_queue (Process* proc);
		Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
		int get_quantum ();
		void set_quantum (int num);
};

class Scheduler_EPRIO: public Scheduler {
	public:
		Scheduler_EPRIO(int prio);
		~Scheduler_EPRIO() {};
		void add_to_queue (Process* proc);
		Process* get_from_queue (Process* cur_proc, deque<Event>* event_queue, int cur_time, int* cur_end_time);
		int get_quantum ();
		void set_quantum (int num);
};

///////////////////////////////////////

int my_random(int up_limit, vector<long>* rand_num, vector<long>::iterator* rand_ite);

int insert_process(vector<Process>* stat_info, Process* proc);

int create_info (MidInfo* info, int start_time, int process_num, int last_time, int prev_state, int next_state,int cb, int ib, int rem, int prio);

void print_info(MidInfo info);

void get_io_utiliz(vector<MidInfo>* info_vec, double* io_utilize);

void print_process(Process proc);

int print_sum(char sched_type, vector<Process>* stat_info, vector<MidInfo>* info_vec);

int print_mid_res(vector<MidInfo>* vec);

int simulation(ifstream* file, vector<long>* rand_num, vector<long>::iterator* rand_ite, deque<Event>* event_queue, vector<Process>* stat_info, vector<MidInfo>* info_vec);

int get_num(char* sched_name);

int init_event_proc(ifstream* file, int max_prio, vector<long>* rand_num, vector<long>::iterator* rand_ite, DES_layer* des);
