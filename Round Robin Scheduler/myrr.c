#include "sched.h"

static void put_prev_task_myrr(struct rq *rq, struct task_struct *p);
static int select_task_rq_myrr(struct task_struct *p, int cpu, int sd_flag, int flags);
static void set_curr_task_myrr(struct rq *rq);
static void task_tick_myrr(struct rq *rq,struct task_struct *p, int oldprio);
static void switched_to_myrr(struct rq *rq, struct task_struct *p);
void init_myrr_rq(struct myrr_rq *myrr_rq);
static void update_curr_myrr(struct rq *rq);
static void enqueue_task_myrr(struct rq *rq, struct task_struct *p, int flags);
static void dequeue_task_myrr(struct rq *rq, struct task_struct *p, int flags);
static void check_preempt_curr_myrr(struct rq *rq, struct task_struct *p,int flags);
struct task_struct *pick_next_task_myrr(struct rq *rq, struct task_struct *prev);
static void prio_changed_myrr(struct rq *rq, struct task_struct *p, int oldprio);

#define MYRR_TIME_SLICE 3

const struct sched_class myrr_sched_class = {
    	.next			= &fair_sched_class,
    	.enqueue_task		= &enqueue_task_myrr,
    	.dequeue_task		= dequeue_task_myrr,
    	.check_preempt_curr	= check_preempt_curr_myrr,
    	.pick_next_task		= pick_next_task_myrr,
    	.put_prev_task		= put_prev_task_myrr,
#ifdef CONFIG_SMP
    	.select_task_rq		= select_task_rq_myrr,
#endif
    	.set_curr_task		= set_curr_task_myrr,
    	.task_tick		= task_tick_myrr,
    	.prio_changed		= prio_changed_myrr,
    	.switched_to		= switched_to_myrr,
    	.update_curr		= update_curr_myrr,
};


void init_myrr_rq (struct myrr_rq *myrr_rq)
{
    	printk(KERN_INFO "***[MYRR] Mysched class is online \n");
    	myrr_rq->nr_running = 0;
    	INIT_LIST_HEAD(&myrr_rq->queue);

}

static void update_curr_myrr(struct rq *rq){
	/*
	if running task use every time slice which it has,
	1. reschedule (change turn in sub run queue
	2. initialize update num
	3. use resched_curr(rq) for rescheduling
	*/
	struct myrr_rq *myrr_rq = &rq->myrr;
	struct sched_myrr_entity *myrr_se = list_entry(myrr_rq->queue.next, struct sched_myrr_entity, run_list);
	struct task_struct *p = container_of(myrr_se, struct task_struct, myrr);

	myrr_se->update_num++;

	printk(KERN_INFO"***[MYRR] update_curr_myrr		pid=%d update_num=%d\n", p->pid, myrr_se->update_num); 
	
	if (myrr_se->update_num > MYRR_TIME_SLICE) {
		myrr_se->update_num = 0;
		
		// TODO: reschedule - change turn in sub rq
		// Delete first and Insert old first
		list_del(&p->myrr.run_list);
		list_add_tail(&p->myrr.run_list, &rq->myrr.queue);
		
		resched_curr(rq);

		printk(KERN_INFO"***[MYRR] update_curr_myrr: Change curr\n"); 
	}

}

static void enqueue_task_myrr(struct rq *rq, struct task_struct *p, int flags) {
	// TODO: Push to sub rq
	printk(KERN_INFO"***[MYRR] Enqueue Start\n");
	
	list_add_tail(&p->myrr.run_list, &rq->myrr.queue);
	rq->myrr.nr_running++;

    printk(KERN_INFO"***[MYRR] Enqueue: success cpu=%d, nr_running=%d, pid=%d\n", cpu_of(rq), rq->myrr.nr_running, p->pid);
	printk(KERN_INFO"***[MYRR] Enqueue End\n");
}

static void dequeue_task_myrr(struct rq *rq, struct task_struct *p, int flags) 
{
	printk(KERN_INFO "***[MYRR] Dequeue: start\n");
	
	if(rq->myrr.nr_running > 0) {
   		/*
    		OSDC Lab. Add code
		1. operate dequeue in sub rq
    		*/
		
		// Dequeue in sub rq
		list_del_init(&p->myrr.run_list);
        	rq->myrr.nr_running--;
        	printk(KERN_INFO"\t***[MYRR] dequeue: success cpu=%d, nr_running=%d, pid=%d\n", cpu_of(rq), rq->myrr.nr_running, p->pid);
    	}
    	else{}

	printk(KERN_INFO "***[MYRR] Dequeue: end\n");
    
}

void check_preempt_curr_myrr(struct rq *rq, struct task_struct *p, int flags) {
	printk(KERN_INFO"***[MYRR] check_preempt_curr_myrr\n");
}

struct task_struct *pick_next_task_myrr(struct rq *rq, struct task_struct *prev) 
{
    struct myrr_rq *myrr_rq = &rq->myrr;
    struct sched_myrr_entity *myrr_se = NULL;
	struct task_struct *p;

	if(rq->myrr.nr_running == 0 ){
		return NULL;
	}
	
	else {
		//TODO: pick next task from sub rq
		update_curr_myrr(rq);
		
		myrr_se = list_entry(myrr_rq->queue.next, struct sched_myrr_entity, run_list);
		p = container_of(myrr_se, struct task_struct, myrr);
		printk(KERN_INFO "\t\t***[MYRR] Pick_next_task: cpu=%d, prev->pid=%d, next_p->pid=%d, nr_running=%d\n", cpu_of(rq), prev->pid, p->pid, rq->myrr.nr_running);
    	printk(KERN_INFO"***[MYRR] Pick Next Task Myrr End\n");	
		return p;
	}
}

void put_prev_task_myrr(struct rq *rq, struct task_struct *p) {
   	printk(KERN_INFO "\t***[MYRR] put_prev_task: do_nothing, p->pid=%d\n", p->pid);
}

int select_task_rq_myrr(struct task_struct *p, int cpu, int sd_flag, int flags) {
	return task_cpu(p);
}

void set_curr_task_myrr(struct rq *rq){
	printk(KERN_INFO"***[MYRR] set_curr_task_myrr\n");
}

void task_tick_myrr(struct rq *rq, struct task_struct *p, int queued) {
	update_curr_myrr(rq);
}

void prio_changed_myrr(struct rq *rq, struct task_struct *p, int oldprio) { 

}

/*This routine is called when a task migrates between classes*/
void switched_to_myrr(struct rq *rq, struct task_struct *p) {
    resched_curr(rq);
}
