#include "linux/kernel.h"
#include "linux/sched.h"

struct task_struct init_task = {
	.state = 0xFFFF,
};
