#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/types.h>
#include <sched.h>

#define SCHED_MYSCHED	7
#define SCHED_MYRR	8
#define SCHED_MYPRIO	9
#define NR_TASKS 	4

#ifdef __x86_64__
#define __NR_sched_setattr		314
#endif

#ifdef __i386__
#define __NR_sched_setattr		351
#endif

struct sched_attr {
	__u32 size;

	__u32 sched_policy;
	__u64 sched_flags;

	/* SCHED_NORMAL, SCHED_BATCH */
	__s32 sched_nice;

	/* SCHED_FIFO, SCHED_RR */
	__u32 sched_priority;

	/* SCHED_DEADLINE */
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;
};

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
	return syscall(__NR_sched_setattr, pid, attr, flags);
}

int main(int argc, char** argv)
{
	pid_t pids, my_pid;
	int nr_proc, my_id;
	int ret;
	struct sched_attr attr;
	unsigned int flags = 0, i;
	char c;
	cpu_set_t mask;

	int max_cpu  = -1;
	int cpu = -1;
	
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);

	pids = getpid();

	if (sched_setaffinity(pids, sizeof(mask), &mask) ) {
		fprintf(stderr, "cpuset failed\n");
		exit(EXIT_FAILURE);
	}
	else {
		printf("cpuset at [0th] cpu in parent process(pid=%d) is succeed\n", pids);
	}

	if (argc != 2) {
		printf("***[NEWCLASS] Need argument: qos_fork {f | n | r}\n");
		exit(1);
	}
	
	c = argv[1][0];
	nr_proc = NR_TASKS;
	
	for (i = 0; i < nr_proc; i++) {

		if ((pids = fork()) < 0) {
			perror("fork error");
			return -1;
		} 

		else if (pids == 0) {
			my_pid = getpid();
			my_id = i;
				
			if (c == 'n') {
				printf("***[NEWCLASS] Select mysched scheduling class \n");
				/* set attributes for SCHED_DEADLINE */
				attr.size = sizeof(attr);
				attr.sched_policy = SCHED_MYSCHED;			
				attr.sched_flags = 0;

				attr.sched_period = 0;
				attr.sched_runtime = 0;
				attr.sched_deadline = 0;

				attr.sched_nice = 0;		// for SCHED_NORMAL and SCHED_BATCH
				attr.sched_priority = 0;	// for SCHED_FIFO and SCHED_RR
				
				ret = sched_setattr(my_pid, &attr, flags);
				if (ret != 0) {
					perror("sched_setattr");
					exit(1);
				}
			}

			else if (c == 'r') {
				printf("***[NEWCLASS] Select myrr scheduling class \n");
				/* set attributes for SCHED_DEADLINE */
				attr.size = sizeof(attr);
				attr.sched_policy = SCHED_MYRR;			
				attr.sched_flags = 0;

				attr.sched_period = 0;
				attr.sched_runtime = 0;
				attr.sched_deadline = 0;

				attr.sched_nice = 0;		// for SCHED_NORMAL and SCHED_BATCH
				attr.sched_priority = 0;	// for SCHED_FIFO and SCHED_RR
				
				ret = sched_setattr(my_pid, &attr, flags);
				if (ret != 0) {
					perror("sched_setattr");
					exit(1);
				}
			}

			else if(c == 'p') {
				printf("***[NEWCLASS] Select myprio scheduling class \n");
				/* set attributes for SCHED_DEADLINE */
				attr.size = sizeof(attr);
				attr.sched_policy = SCHED_MYPRIO;			
				attr.sched_flags = 0;

				attr.sched_period = 0;
				attr.sched_runtime = 0;
				attr.sched_deadline = 0;

				attr.sched_nice = 0;		// for SCHED_NORMAL and SCHED_BATCH
				attr.sched_priority = 0;	// for SCHED_FIFO and SCHED_RR
				
				ret = sched_setattr(my_pid, &attr, flags);
				if (ret != 0) {
					perror("sched_setattr");
					exit(1);
				}
			}

			else if (c == 'f')
				printf("***[NEWCLASS] Select CFS class \n");

			else {
				printf("***[NEWCLASS] Select undefined class \n");
				exit(1);
			}

			CPU_ZERO(&mask);
			CPU_SET(1, &mask);

			if (sched_setaffinity(my_pid, sizeof(mask), &mask)) {
				printf("CPUset failed\n");
				fprintf(stderr, "cpuset failed\n");
				exit(EXIT_FAILURE);
			}

			else {
				printf("cpuset at [1st] cpu in child process(pid=%d) is succeed\n", my_pid);
			}

			sleep(1);
				
			/* child process work */					
			int j = 0;
			for(j = 0; j < 20; j++) {
				int i = 0;
				int result = 0;
				for(i = 0; i < 200000000; i++)
				{
					result += 1;
				}
				printf("pid=%d:\tresult=%d\n", my_pid, result);
				//sleep(1);
			}

			exit(1);

		}

		printf("Child's PID = %d\n", pids);
	}
	
	printf("forking %d tasks is completed\n", nr_proc);

	return 0;
}
