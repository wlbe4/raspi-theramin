/**
 * @file tof1029.c
 * @brief Functions and data related to the TOF1020 char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Lior Weintraub
 * @date 2025-03-08
 * @copyright Copyright (c) 2025
 *
 */

#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "tof1020.h"


int tof1020_major =   0; // use dynamic major
int tof1020_minor =   0;

MODULE_AUTHOR("Lior Weintraub");
MODULE_LICENSE("Dual BSD/GPL");

struct tof1020_dev tof1020_device;

int tof1020_open(struct inode *inode, struct file *filp)
{
    struct tof1020_dev *dev; /* device information */
    PDEBUG("open");

    dev = container_of(inode->i_cdev, struct tof1020_dev, cdev);
    filp->private_data = dev; /* for other methods */
 
    return 0;
}

int tof1020_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    return 0;
}

ssize_t tof1020_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t bytes_to_copy = 2;
    uint16_t val = 0x1234;
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    if (*f_pos >= 2 ) {
        return 0;
    }
    //struct tof1020_dev *dev = filp->private_data;
    if(copy_to_user(buf,&val,bytes_to_copy)) {
        return -EFAULT;
    }
    *f_pos = bytes_to_copy;
    return bytes_to_copy;
}

struct file_operations tof1020_fops = {
    .owner =    THIS_MODULE,
    .read =     tof1020_read,
    .open =     tof1020_open,
    .release =  tof1020_release,
};

static int tof1020_setup_cdev(struct tof1020_dev *dev)
{
    int err, devno = MKDEV(tof1020_major, tof1020_minor);

    cdev_init(&dev->cdev, &tof1020_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &tof1020_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding tof1020 cdev", err);
    }
    return err;
}



int tof1020_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, tof1020_minor, 1,
            "tof1020");
    tof1020_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", tof1020_major);
        return result;
    }
    memset(&tof1020_device,0,sizeof(struct tof1020_dev));


    result = tof1020_setup_cdev(&tof1020_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void tof1020_cleanup_module(void)
{
    dev_t devno = MKDEV(tof1020_major, tof1020_minor);

    cdev_del(&tof1020_device.cdev);

    unregister_chrdev_region(devno, 1);
}



module_init(tof1020_init_module);
module_exit(tof1020_cleanup_module);
