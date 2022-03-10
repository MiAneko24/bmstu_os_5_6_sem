#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/init_task.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Serova Maria");

static int __init md_init(void)
{
    struct task_struct *task = &init_task;
    do
    {
        printk(KERN_INFO "pid = %d, name = %s, state = %ld, prio = %d, ppid = %d, name_p = %s\n",
        task->pid, task->comm, task->state, task->prio, task->parent->pid, task->parent->comm);
    } while ((task = next_task(task)) != &init_task);
    printk(KERN_INFO "Current pid = %d, name = %s, state = %ld, prio = %d, ppid = %d, name_p = %s\n",
        current->pid, current->comm, current->state, current->prio, current->parent->pid, current->parent->comm);
    printk(KERN_INFO "Loaded md\n");
    return 0;
}

static void __exit md_exit(void)
{
    printk(KERN_INFO "Current pid = %d, name = %s, state = %ld, prio = %d, ppid = %d, name_p = %s\n",
        current->pid, current->comm, current->state, current->prio, current->parent->pid, current->parent->comm);
    
    printk(KERN_INFO "Exit\n");
}

module_init(md_init);
module_exit(md_exit);