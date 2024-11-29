#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "scull"
#define BUF_LEN 80

static int major;
static char msg[BUF_LEN];
static struct cdev scull_cdev;
static int dev_open = 0;

static int device_open(struct inode *inode, struct file *file) {
    if (dev_open)
        return -EBUSY;

    dev_open++;
    try_module_get(THIS_MODULE);
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    dev_open--;
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset) {
    int bytes_read = 0;

    if (*msg == 0)
        return 0;

    while (len && *msg) {
        put_user(*(msg++), buffer++);
        len--;
        bytes_read++;
    }

    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t len, loff_t *offset) {
    int bytes_written = 0;

    if (len > BUF_LEN - 1)
        return -EINVAL;

    if (copy_from_user(msg, buffer, len))
        return -EFAULT;

    msg[len] = '\0';
    bytes_written = len;

    return bytes_written;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
};

static int __init scull_init(void) {
    dev_t dev;
    int result;

    result = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (result < 0) {
        printk(KERN_ALERT "scull: failed to register device\n");
        return result;
    }

    major = MAJOR(dev);

    cdev_init(&scull_cdev, &fops);
    scull_cdev.owner = THIS_MODULE;

    result = cdev_add(&scull_cdev, dev, 1);
    if (result) {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT "scull: failed to add cdev\n");
        return result;
    }

    printk(KERN_INFO "scull: registered with major number %d\n", major);
    return 0;
}

static void __exit scull_exit(void) {
    cdev_del(&scull_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
    printk(KERN_INFO "scull: unregistered device\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A Simple Character Device Driver");
MODULE_VERSION("0.1");

module_init(scull_init);
module_exit(scull_exit);
