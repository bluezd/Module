#include <linux/init.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/syscalls.h>

char c[] = "times";
#define MAX_SIZE 1000

static int __init utime_stime_test(void)
{      
        int count = 0;
        struct file *fp;

        char buffer[MAX_SIZE];

        struct list_head *prev;
        struct task_struct *p;
        struct task_struct *first = &init_task;

        mm_segment_t fs;

        fp = filp_open("/tmp/MODULE_tmp",O_CREAT | O_RDWR ,0644);

        fs = current_thread_info()->addr_limit;

        current_thread_info()->addr_limit = KERNEL_DS;

        if (IS_ERR(fp)) {
                printk(KERN_INFO"create file error/n");
                return 1;
        }
       
        if (!(fp->f_mode & FMODE_WRITE)){
                printk(KERN_INFO"No write permission\n");
                return 1;
        }
       
        printk(KERN_INFO "starting searehing the process\n");

        list_for_each(prev,&first->tasks){
                p =  list_entry(prev, struct task_struct, tasks);
               
                if (!strcmp(p->comm, c)) {
                        sprintf(buffer,"Found the process : \n%s pid = %d\n -> utime = %lu, stime = %lu",p->comm,p->pid,(unsigned long)p->utime, (unsigned long)p->stime);

                        vfs_write(fp, buffer, strlen(buffer), &fp->f_pos);
                        memset(buffer,0,MAX_SIZE);


                        while (count < 100) {
                                p->utime = 0x1111111100000000;
                                p->stime = 0x0;
                               
                                ++count;
                        }


                        // sprintf(buffer, "\nAfter setting:\n -> utime = %lu, stime = %lu\n",(unsigned long)p->utime, (unsigned long)p->stime);

                        //vfs_write(fp, buffer, strlen(buffer), &fp->f_pos);

                        break;
                }
        }

        return 0;
}

static void utime_stime_exit(void)
{
}

module_init(utime_stime_test);
module_exit(utime_stime_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Test");
MODULE_AUTHOR("Dong Zhu");
