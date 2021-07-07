#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<asm/uaccess.h>

MODULE_LICENSE("GPL");

#define SUCCESS 0
#define DEVICE_NAME "mydevice" 

static int Major;
static int Device_Open=0;

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset);
static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);

struct file_operations fops ={
    .read = device_read,
    .open = device_open,
    .release = device_release
};

int init_mydriver(void){
    Major = register_chrdev(0,DEVICE_NAME,&fops);
    if(Major < 0){
        printk(KERN_ALERT "Registering failed with %d\n", Major);
        return Major;
    }
    printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
    return SUCCESS;
}

void cleanup_mydriver(void) {
    unregister_chrdev(Major, DEVICE_NAME);
    printk(KERN_ALERT "Сleanup_module OK \n");
}

static int device_open(struct inode *inode, struct file *file){
    if (Device_Open)
        return -EBUSY;
    Device_Open++;
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file){
    Device_Open--;
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    static unsigned int counter = 0;
    counter++;
    raw_copy_to_user(buffer, &counter, sizeof(unsigned int));
    return counter;
}

module_init(init_mydriver);
module_exit(cleanup_mydriver);