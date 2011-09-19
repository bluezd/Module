#include<linux/init.h>
#include<linux/module.h>
#include <linux/cpu.h>

static int __cpuinit foobar_cpu_callback(struct notifier_block *nfb,unsigned long action, void *hcpu);

static struct notifier_block __cpuinitdata foobar_cpu_notifer =
{
       .notifier_call = foobar_cpu_callback,
};


static void __init _cpu_hotplug_init(void)
{
	register_cpu_notifier(&foobar_cpu_notifer);
}

static int foobar_dead_action(unsigned int cpu)
{
        printk(KERN_NOTICE"cpu %d dead ...\n",cpu);
        return 0;
}

static int __cpuinit foobar_cpu_callback(struct notifier_block *nfb,unsigned long action, void *hcpu)
{
        unsigned int cpu = (unsigned long)hcpu;

        switch (action) {
              case CPU_ONLINE:
              case CPU_ONLINE_FROZEN:
                      //foobar_online_action(cpu);
                      
                      break;
              case CPU_DEAD:
              case CPU_DEAD_FROZEN:
                        foobar_dead_action(cpu);
                        break;
        }
        return NOTIFY_OK;
}


static void __exit _cpu_hotplug_exit(void)
{
	printk(KERN_NOTICE"_cpu_hotplug_exit\n");
}

module_init(_cpu_hotplug_init);
module_exit(_cpu_hotplug_exit);
