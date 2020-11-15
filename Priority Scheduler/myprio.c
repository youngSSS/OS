#include "sched.h"
#include <linux/timer.h>

static void put_prev_task_myprio(struct rq *rq, struct task_struct *p);
static int select_task_rq_myprio(struct task_struct *p, int cpu, int sd_flag, int flags);
static void set_curr_task_myprio(struct rq *rq);
static void task_tick_myprio(struct rq *rq,struct task_struct *p, int oldprio);
static void switched_to_myprio(struct rq *rq, struct task_struct *p);
void init_myprio_rq(struct myprio_rq *myprio_rq);
static void update_curr_myprio(struct rq *rq);
static void enqueue_task_myprio(struct rq *rq, struct task_struct *p, int flags);
static void dequeue_task_myprio(struct rq *rq, struct task_struct *p, int flags);
static void check_preempt_curr_myprio(struct rq *rq, struct task_struct *p,int flags);
struct task_struct *pick_next_task_myprio(struct rq *rq, struct task_struct *prev);
static void prio_changed_myprio(struct rq *rq, struct task_struct *p, int oldprio);

unsigned long time_to_raise_priority = HZ;

const struct sched_class myprio_sched_class = {
    	.next			= &mysched_sched_class,
    	.enqueue_task		= &enqueue_task_myprio,
    	.dequeue_task		= dequeue_task_myprio,
    	.check_preempt_curr	= check_preempt_curr_myprio,
    	.pick_next_task		= pick_next_task_myprio,
    	.put_prev_task		= put_prev_task_myprio,
#ifdef CONFIG_SMP
    	.select_task_rq		= select_task_rq_myprio,
#endif
    	.set_curr_task		= set_curr_task_myprio,
    	.task_tick		= task_tick_myprio,
    	.prio_changed		= prio_changed_myprio,
    	.switched_to		= switched_to_myprio,
    	.update_curr		= update_curr_myprio,
};


void init_myprio_rq (struct myprio_rq *myprio_rq)
{
    	printk(KERN_INFO "***[MYPRIO] Myprio class is online \n");
    	myprio_rq->nr_running = 0;
    	INIT_LIST_HEAD(&myprio_rq->queue);

}

static void update_curr_myprio(struct rq *rq) {
	struct myprio_rq *myprio_rq = &rq->myprio;
	struct sched_myprio_entity *temp_se = NULL;
	struct sched_myprio_entity *highest_prio_se = NULL;
	struct task_struct *highest_prio_p;
	struct sched_myprio_entity *curr_se = list_entry(myprio_rq->queue.next, struct sched_myprio_entity, run_list);
	struct task_struct *curr_p = container_of(curr_se, struct task_struct, myprio);

	/* 
	TODO: update current task
	1. Traverse the rq and pick highest priority task
	2. If highest priority task is not curr, change it'position in rq to curr's position (=first in rq)
	3. Set old curr's wait time, Set old curr's priority to original priority
	4. Ask rescheduling
	*/ 

	// Step 1
	int temp_priority = 10000;
	list_for_each_entry(temp_se, &rq->myprio.queue, run_list) {
	if (temp_priority > temp_se->priority) {
		temp_priority = temp_se->priority;
		highest_prio_se = temp_se;
		}
	}
		
	highest_prio_p = container_of(highest_prio_se, struct task_struct, myprio);

	if (curr_se->priority > highest_prio_se->priority) {			
		// Step 2
		list_del(&highest_prio_p->myprio.run_list);
		list_add(&highest_prio_p->myprio.run_list, &rq->myprio.queue);
		
		printk(KERN_INFO "\t***[MYPRIO] update_curr_myprio: Old Curr=%d, Old Prio=%d, New Curr=%d, New Prio=%d\n", curr_p->pid, curr_se->priority, highest_prio_p->pid, highest_prio_se->priority);
		
		// Step 3
		curr_se->wait_time = jiffies;
		curr_se->priority = curr_p->pid;

		// Step 4
		resched_curr(rq);
	}

	else {
		printk(KERN_INFO "\t***[MYPRIO] update_curr_myprio: curr is not changed\n"); 
	}		
}

// Push task into the rq, Give priority
static void enqueue_task_myprio(struct rq *rq, struct task_struct *p, int flags) {
	struct sched_myprio_entity *myprio_se = &p->myprio;

	printk(KERN_INFO "***[MYPRIO] Enqueue Start\n");
	
	myprio_se->wait_time = jiffies;
	myprio_se->priority = p->pid;

	list_add_tail(&p->myprio.run_list, &rq->myprio.queue);
	
	//TODO: Check priority
	update_curr_myprio(rq);
	
	rq->myprio.nr_running++;

	printk(KERN_INFO "***[MYPRIO] Enqueue: success cpu=%d, nr_running=%d, pid=%d, prio=%d\n", cpu_of(rq), rq->myprio.nr_running, p->pid, myprio_se->priority);
	printk(KERN_INFO "***[MYPRIO] Enqueue End\n");
}

static void dequeue_task_myprio(struct rq *rq, struct task_struct *p, int flags) 
{
	struct sched_myprio_entity *myprio_se = &p->myprio;

	printk(KERN_INFO "***[MYPRIO] Dequeue: start\n");
	
	if(rq->myprio.nr_running > 0) {
		// Dequeue from rq
		list_del_init(&p->myprio.run_list);
		rq->myprio.nr_running--;
		printk(KERN_INFO "***[MYPRIO] dequeue: success cpu=%d, nr_running=%d, pid=%d, prio=%d\n", cpu_of(rq), rq->myprio.nr_running, p->pid, myprio_se->priority);
    	}
    	else {}

	printk(KERN_INFO "***[MYPRIO] Dequeue: end\n");
}

void check_preempt_curr_myprio(struct rq *rq, struct task_struct *p, int flags) {
	printk(KERN_INFO "***[MYPRIO] check_preempt_curr_myprio\n");
}

struct task_struct *pick_next_task_myprio(struct rq *rq, struct task_struct *prev) 
{
	struct myprio_rq *myprio_rq = &rq->myprio;
	struct sched_myprio_entity *selected_se = NULL;
	struct task_struct *selected_p;

	if(rq->myprio.nr_running == 0 ){
		return NULL;
	}
	
	else {	
		selected_se = list_entry(myprio_rq->queue.next, struct sched_myprio_entity, run_list);
		selected_p = container_of(selected_se, struct task_struct, myprio);

		printk(KERN_INFO "***[MYPRIO] Pick_next_task: cpu=%d, prev->pid=%d, next_p->pid=%d, next_p prio=%d, nr_running=%d\n", cpu_of(rq), prev->pid, selected_p->pid, selected_se->priority, rq->myprio.nr_running);
		printk(KERN_INFO "***[MYPRIO] Pick Next Task Myprio End\n");

		return selected_p;
	}
}

void put_prev_task_myprio(struct rq *rq, struct task_struct *p) {
   	printk(KERN_INFO "\t***[MYPRIO] put_prev_task: do_nothing, p->pid=%d\n", p->pid);
}

int select_task_rq_myprio(struct task_struct *p, int cpu, int sd_flag, int flags) {
	return task_cpu(p);
}

void set_curr_task_myprio(struct rq *rq){
	printk(KERN_INFO "***[MYPRIO] set_curr_task_myprio\n");
}

// Check task wating time. if wating time is too long, raise task's priority
void task_tick_myprio(struct rq *rq, struct task_struct *p, int queued) {
	struct sched_myprio_entity *myprio_se = NULL;
	struct sched_myprio_entity *curr_se = &p->myprio;
	struct task_struct *temp_p;

	list_for_each_entry(myprio_se, &rq->myprio.queue, run_list) {
		if (time_after(jiffies - myprio_se->wait_time, time_to_raise_priority)) {
			temp_p = container_of(myprio_se, struct task_struct, myprio);
			if (temp_p->pid != p->pid) {
				printk(KERN_INFO "\t\t***[MYPRIO] priority raise: pid=%d, old prio=%d, new prio=%d\n", temp_p->pid, myprio_se->priority, myprio_se->priority - 1);
				myprio_se->priority--;
				myprio_se->wait_time = jiffies;
				if (myprio_se->priority < curr_se->priority)
					update_curr_myprio(rq);
			}
		}
	}

}

void prio_changed_myprio(struct rq *rq, struct task_struct *p, int oldprio) { 

}

/*This routine is called when a task migrates between classes*/
void switched_to_myprio(struct rq *rq, struct task_struct *p) {
    resched_curr(rq);
}
