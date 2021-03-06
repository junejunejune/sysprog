#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

static struct dentry *dir, *inputdir, *ptreedir;
static struct task_struct *curr;
struct debugfs_blob_wrapper *myblob;

char stats[8192];
int i;

static ssize_t write_pid_to_input(struct file *fp, 
                                const char __user *user_buffer, 
                                size_t length, 
                                loff_t *position)
{
        pid_t input_pid;
        struct pid * pid;

        for (i = 0; i < 8192; i++){
                stats[i] = '\0';
        }

        sscanf(user_buffer, "%u", &input_pid);
        pid = find_get_pid(input_pid);
        // curr = pid_task(input_pid, PIDTYPE_PID); // Find task_struct using input_pid. Hint: pid_task
        curr = pid_task(pid, PIDTYPE_PID); // Find task_struct using input_pid. Hint: pid_task

        // Tracing process tree from input_pid to init(1) process
        
        while(1) {
            if (curr->pid == 1) break;
            length += sprintf(stats + length, "%s (%d)\n", curr->comm, curr->pid);
            curr = curr->real_parent;
        }

        // Make Output Format string: process_command (process_id)
        length += sprintf(stats + length, "%s (%d)\n", curr->comm, curr->pid);

        return length;
}

static const struct file_operations dbfs_fops = {
        .write = write_pid_to_input,
};

static int __init dbfs_module_init(void)
{

        int stats_size;
        int struct_size;

        // Implement init module code

        dir = debugfs_create_dir("ptree", NULL);

        /*
        if (!dir) {
                printk("Cannot create ptree dir\n");
                return -1;
        }
        */

        struct_size = sizeof(struct debugfs_blob_wrapper);
        stats_size = 8192 * sizeof(char);

        /*
        if (stats == NULL) {
                printk("Could not allocate mem for data\n");
                return -ENOMEM;
        }
        */


        myblob = (struct debugfs_blob_wrapper *) kmalloc(struct_size, GFP_KERNEL);

        /*
        if (myblob == NULL) {
                printk("Could not allocate mem for blob\n");
                kfree(stats);
                return -ENOMEM;
        }
        */

        myblob->data = (void *) stats;
        myblob->size = (unsigned long) stats_size;

        inputdir = debugfs_create_file("input", 0644, dir, NULL, &dbfs_fops);
        ptreedir = debugfs_create_blob("ptree", 0644, dir, myblob); // Find suitable debugfs API

        /*
        if (!ptreedir) {
                printk("error creating int file");
                kfree(stats);
                kfree(myblob);
                return (-ENODEV);
        }
        */

        return 0;
}

static void __exit dbfs_module_exit(void)
{
        // Implement exit module code
        kfree(myblob);
        debugfs_remove(ptreedir);
        debugfs_remove(inputdir);
        debugfs_remove_recursive(dir);
}

module_init(dbfs_module_init);
module_exit(dbfs_module_exit);
