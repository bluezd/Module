#include <linux/init.h>	
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/module.h>	
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/syscalls.h>

char c[] = "firefox";
#define MAX_SIZE 1000

static int __init utime_stime_test(void)
{
        struct file *fp;

        ssize_t ret;

        char buffer[MAX_SIZE];

	struct list_head *prev;
	struct task_struct *p;
	struct task_struct *first = &init_task;

        mm_segment_t fs;

        loff_t pos = 0;

        fp = filp_open("/tmp/MODULE_tmp",O_CREAT | O_RDWR ,0644);

        fs = current_thread_info()->addr_limit;

        current_thread_info()->addr_limit = KERNEL_DS;

        if (IS_ERR(fp)) {
                printk(KERN_INFO"create file error/n");
                return ;
        }

        if (!(fp->f_mode & FMODE_WRITE)){
                printk(KERN_INFO"No write permission\n");
                return;
        }

	printk(KERN_INFO "starting searehing the process\n");

	list_for_each(prev,&first->tasks){
		p =  list_entry(prev, struct task_struct, tasks);

		if (!strcmp(p->comm, c)) {
			//printk(KERN_INFO "Found the Process %s , PID = %d\n", p->comm, p->pid);
                        sprintf(buffer,"Found the process : \n%s pid = %d\n -> utime = %lu, stime = %lu",p->comm,p->pid,(unsigned long)p->utime, (unsigned long)p->stime);

                        vfs_write(fp, buffer, strlen(buffer), &fp->f_pos);
                        memset(buffer,0,MAX_SIZE);

			// printk(KERN_INFO "\t utime = %lu, stime = %lu",(unsigned long)p->utime, (unsigned long)p->stime);


			// p->utime = 0xFFFFFFFF00000000;
			p->utime = (cputime_t)0x1000000000000000;
			p->stime = (cputime_t)0x00000000FFFFFFFF;

			sprintf(buffer, "\nAfter setting:\n -> utime = %lu, stime = %lu\n",(unsigned long)p->utime, (unsigned long)p->stime);

                        vfs_write(fp, buffer, strlen(buffer), &fp->f_pos);

			break;
		}
	}
}

static int __exit utime_stime_exit(void)
{}

module_init(utime_stime_test);
module_exit(utime_stime_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Test");
MODULE_AUTHOR("bluezd");
