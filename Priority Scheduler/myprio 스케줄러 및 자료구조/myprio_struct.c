
/* 
스케줄러 우선순위 
rt > mysched > myrr > myprio > fair > idle
*/

// <include/linux/sched.h>
struct sched_myprio_entity {
	struct list_head run_list;
	unsigned int priority;
	unsigned int age;
	unsigned long burst_time;
	unsigned long tot_wait_time;
	unsigned long turn_around_time;
	unsigned long rq_wait_time;
};

// <kernel/sched/sched.h>
struct myprio_rq {
	struct list_head queue;
	unsigned int nr_running;
};