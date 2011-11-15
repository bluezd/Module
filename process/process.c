#include<linux/init.h>
#include<linux/list.h>
#include<linux/module.h>
#include <linux/string.h>
#include<linux/sched.h>
#include<linux/fcntl.h>
#include <linux/fs.h>
#include<linux/syscalls.h>

#define MAX_SIZE 1000

static void process_list(void)
{
	int count=0;
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

	//printk(KERN_INFO"thread_info -> addr_limit = %p\n",fs);

	current_thread_info()->addr_limit = KERNEL_DS;
	//printk(KERN_INFO"thread_info -> addr_limit = %p\n",current_thread_info()->addr_limit);

	if (IS_ERR(fp)) {
		printk(KERN_INFO"create file error/n");
                return ;
	} 

	if (!(fp->f_mode & FMODE_WRITE)){
		printk(KERN_INFO"No write permission\n");
                return;
	}

	printk(KERN_INFO"starting list the process\n");
	list_for_each(prev,&first->tasks)
	{
		p = list_entry(prev,struct task_struct,tasks);
		count++;

		
		sprintf(buffer,"%s pid = %d \n\t-->prio = %d static_prio = %d normal_prio = %d\n",p->comm,p->pid,p->prio, p->static_prio, p->normal_prio);

		vfs_write(fp, buffer, strlen(buffer), &fp->f_pos);
		memset(buffer,0,MAX_SIZE); 
 
                /*if (!(strcmp(p->comm,"firefox"))){
			sprintf(buffer,"Found the process : %s pid = %d\n",p->comm,p->pid);

			printk(KERN_INFO"%s\n",buffer);

			ret = vfs_write(fp, buffer, strlen(buffer), &fp->f_pos);

			if (ret == strlen(buffer))
				printk(KERN_INFO"write done\n");
			else {
				printk(KERN_INFO"ret = %d\n",ret);
				printk(KERN_INFO"write error\n");
			}

			//ret = fp->f_op->write(fp, buffer, strlen(buffer), &fp->f_pos);

			//printk(KERN_INFO"%d\t%s\n", p->pid, p->comm );
		}*/

	}

        //sprintf(buffer,"current : %s pid = %d\n",current->comm,current->pid);

        //vfs_write(fp, buffer, strlen(buffer), &fp->f_pos);
        //memset(buffer,0,MAX_SIZE);

	//printk(KERN_INFO"total processes %d\n",count); 

	filp_close(fp, NULL);
}

static void process_exit(void)
{
	printk(KERN_INFO"process module exit\n");
}

module_init(process_list);
module_exit(process_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Test");
MODULE_AUTHOR("bluezd");
