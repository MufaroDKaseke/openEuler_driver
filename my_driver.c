#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "brightness"
#define CLASS_NAME "brightness_class"

static int major_number;
static int brightness_level = 100; // Default brightness: 100%
static struct class* brightness_class = NULL;
static struct device* brightness_device = NULL;

static void adjust_brightness(int level) {
    printk(KERN_INFO "Adjusting brightness to %d%%\n", level);
    // Extend this to interact with frame buffer or hardware
}

static int dev_open(struct inode* inodep, struct file* filep) {
    printk(KERN_INFO "Brightness device opened\n");
    return 0;
}

static ssize_t dev_write(struct file* filep, const char* buffer, size_t len, loff_t* offset) {
    int new_level;
    if (kstrtoint_from_user(buffer, len, 10, &new_level) != 0) {
        return -EFAULT;
    }
    if (new_level < 0 || new_level > 100) {
        printk(KERN_WARNING "Invalid brightness level: %d\n", new_level);
        return -EINVAL;
    }
    brightness_level = new_level;
    adjust_brightness(brightness_level);
    return len;
}

// File operations structure
static struct file_operations fops = {
    .open = dev_open,
    .write = dev_write,
};

static int __init brightness_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Failed to register a major number\n");
        return major_number;
    }
    brightness_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(brightness_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(brightness_class);
    }
    brightness_device = device_create(brightness_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(brightness_device)) {
        class_destroy(brightness_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(brightness_device);
    }
    printk(KERN_INFO "Brightness driver initialized\n");
    return 0;
}

static void __exit brightness_exit(void) {
    device_destroy(brightness_class, MKDEV(major_number, 0));
    class_unregister(brightness_class);
    class_destroy(brightness_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Brightness driver removed\n");
}

module_init(brightness_init);
module_exit(brightness_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("fdubejr");
MODULE_DESCRIPTION("simple brightness control driver, maYbE");
MODULE_VERSION("0.1")