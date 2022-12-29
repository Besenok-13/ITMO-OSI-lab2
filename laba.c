#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <asm/thread_info.h>
#include <asm/processor.h>
#include <linux/sched.h>
#include <linux/pci.h> 
#include <linux/mutex.h>

#define MAX_DEBUGFS_SIZE 100 // max bite number
#define DEBUGFS_INTERFACE_DIR "laba"                            // was ovo
#define DEBUGFS_THREAD_STRUCTURE_INTERFACE_FILE "thread_struct" // was ts
#define DEBUGFS_PCI_DEV_INTERFACE_FILE "pci_dev"

static char thread_struct_buffer[MAX_DEBUGFS_SIZE];
static unsigned long thread_struct_buffer_size = 0;

static char pci_dev_buffer[MAX_DEBUGFS_SIZE];
static unsigned long pci_dev_buffer_size = 0;

struct mutex mutex_thread_struct;
struct mutex mutex_pci_dev;


// Thread srtuct info interface methods:

ssize_t thread_struct_read_interface(struct file *file, char __user *buff, size_t count, loff_t *offset)
{
    pr_info("Reading TS Info"); //logging

    mutex_lock(&mutex_thread_struct);

    static int read_status = 0;

    if (read_status != 0) // vernuli kol-vo prochitan bytes 
    {
        read_status = 0; //prerivanie 
        return 0;
    }

    printk("TS read input : %s\n", thread_struct_buffer); //logging 
    long seeked_pid = 1; 
    kstrtol(thread_struct_buffer, 10, &seeked_pid); // kernel - string to long i peredayom v seeked_pid thread_struct_buffer
    printk("TS read input atoi: %d\n", seeked_pid);
    struct task_struct *task; // lezhit ssilka
    int found_flag = 0;
    for_each_process(task)
    {
        if (task->pid == seeked_pid)
        {
            found_flag = 1;
            break;
        }
    }
    if (found_flag == 1)
    {
        struct thread_struct found_thread = task->thread; // target thread structure //attribut tak kak y nas tyt ssilka - strelku delaem
 
        // target fields values:
        unsigned long sp = found_thread.sp;
        unsigned short es = found_thread.es;
        unsigned short ds = found_thread.ds;
        unsigned long iobm = found_thread.io_bitmap;
        unsigned long thread_struct_error_code = found_thread.error_code;

        // values to string format:
        const char fields_values_str[MAX_DEBUGFS_SIZE];
        const char format_answer[] = "Stack pointer: %u\nES register: %u\nDS register: %u\nIO: %u\nError code: %u\n";
        size_t string_size = snprintf(NULL, 0, format_answer, sp, es, ds, iobm, thread_struct_error_code) + 1;
        snprintf(fields_values_str, string_size, format_answer, sp, es, ds, iobm, thread_struct_error_code);
        copy_to_user(buff, fields_values_str, string_size);
        read_status = string_size; // trebyet kol-vo prochitanih byte trebovaniye func
    }
    else
    {
        copy_to_user(buff, "Nothing found\n", 15);
        read_status = 0;
    }

    mutex_unlock(&mutex_thread_struct);
    return read_status;
}

ssize_t thread_struct_write_interface(struct file *file, const char __user *buff, size_t count, loff_t *offset)
{
    pr_info("Writing PID to explore TS"); //loging
    if (count <= MAX_DEBUGFS_SIZE)
        thread_struct_buffer_size = count;
    else
        thread_struct_buffer_size = MAX_DEBUGFS_SIZE;
    copy_from_user(thread_struct_buffer, buff, thread_struct_buffer_size); //kopiryem ot polzovatelya nam
    return thread_struct_buffer_size;
}



// Pci dev info interface methods:

ssize_t pci_dev_read_interface(struct file *file, char __user *buff, size_t count, loff_t *offset)
{
    pr_info("Reading PCI Info"); //logging

    mutex_lock(&mutex_pci_dev);

    static int read_status = 0;

    if (read_status != 0) // vernuli kol-vo prochitan bytes 
    {
        read_status = 0; //prerivanie 
        return 0;
    }

    printk("PD read input : %s\n", pci_dev_buffer); //logging 
    long seeked_dev_id = 1; 
    kstrtol(pci_dev_buffer, 10, &seeked_dev_id); // kernel - string to long i peredayom v seeked_pid thread_struct_buffer
    printk("PD read input atoi: %d\n", seeked_dev_id);
    struct pci_dev *device = NULL;
    int found_flag = 0;
    for_each_pci_dev(device) {
        if(device->device == seeked_dev_id) {
            found_flag = 1;
        }
    }
    if (found_flag == 1)
    {
        unsigned int devfn = device->devfn;
        unsigned short vendor = device->vendor;
        unsigned short device_id = device->device;
        void * sysdata = device->sysdata;

        // values to string format:
        const char fields_values_str[MAX_DEBUGFS_SIZE];
        const char format_answer[] = "Encoded device & function index: %u\nVendor id: %u\nDevice id: %u\nHook for sys-specific extension: %p\n";
        size_t string_size = snprintf(NULL, 0, format_answer, devfn, vendor, device_id, sysdata) + 1;
        snprintf(fields_values_str, string_size, format_answer, format_answer, devfn, vendor, device_id, sysdata);
        copy_to_user(buff, fields_values_str, string_size);
        read_status = string_size; // trebyet kol-vo prochitanih byte trebovaniye func
    }
    else
    {
        copy_to_user(buff, "Nothing found\n", 15);
        read_status = 0;
    }

    mutex_unlock(&mutex_pci_dev);
    return read_status;
}

ssize_t pci_dev_write_interface(struct file *file, const char __user *buff, size_t count, loff_t *offset)
{
    pr_info("Writing PID to explore TS"); //loging
    if (count <= MAX_DEBUGFS_SIZE)
        pci_dev_buffer_size = count;
    else
        pci_dev_buffer_size = MAX_DEBUGFS_SIZE;
    copy_from_user(pci_dev_buffer, buff, pci_dev_buffer_size); //kopiryem ot polzovatelya nam
    return pci_dev_buffer_size;
}


// file interfaces

static const struct file_operations thread_struct_file_interface = { //pereopredelyaem chtenie i zapis
    .owner = THIS_MODULE,
    .read = thread_struct_read_interface,
    .write = thread_struct_write_interface
};

static const struct file_operations pci_dev_file_interface = { //pereopredelyaem chtenie i zapis
    .owner = THIS_MODULE,
    .read = pci_dev_read_interface,
    .write = pci_dev_write_interface
}; 


// module init

static int __init initer(void)
{
    pr_info("Initializing debugfs thread_struct and iio_map info interface");//loging
    struct dentry *interface_dir = debugfs_create_dir(DEBUGFS_INTERFACE_DIR, NULL); //sozdayom podpapku
    debugfs_create_file(DEBUGFS_THREAD_STRUCTURE_INTERFACE_FILE, 0777, interface_dir, NULL, &thread_struct_file_interface); //sozdayom file
    debugfs_create_file(DEBUGFS_PCI_DEV_INTERFACE_FILE, 0777, interface_dir, NULL, &pci_dev_file_interface); //sozdayom file
    //suda nygno bydet dobavit file
    return 0;
}

module_init(initer); // sozdayom kusok yadra 
module_exit(goodbye);

MODULE_LICENSE("GPL");