#include<linux/init.h>
#include<linux/list.h>
#include<linux/module.h>
#include<linux/sched.h>

static void process_list(void)
{
	int count=0;

	struct list_head *prev;
	struct task_struct *p;
        struct task_struct *first = &init_task;

	printk(KERN_INFO"starting list the process\n");
	list_for_each(prev,&first->tasks)
	{
		p = list_entry(prev,struct task_struct,tasks);
		count++;
		printk(KERN_INFO"%d\t%s\n", p->pid, p->comm );
	}
	printk(KERN_INFO"total processes %d\n",count);
}

static void process_exit(void)
{
	printk(KERN_INFO"process module exit\n");
}

module_init(process_list);
module_exit(process_exit);
