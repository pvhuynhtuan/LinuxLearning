/*
* sudo dmesg -W                 ==> Show the kernel log
* sudo insmod <.ko file>        ==> install the driver
* sudo rmmod <.ko file>         ==> Remove the driver
*/

#include <linux/module.h> /* Define module_init(), module_exit() */
#include <linux/fs.h> /* define alloc_chrdev_region(), register_chrdev_region() */

#define DRIVER_AUTHOR   "huynhtuan pvhuynhtuan@gmail.com"
#define DRIVER_DESC     "This is the example for driver module - Major/minor number"
#define DRIVER_VERS     "1.0"

struct m_foo_dev {
    dev_t dev_num;
} mdev;

/* Constructor */
static int __init chdev_init(void)
{
    pr_info("MajorMinorExample.ko > Hello world kernel module!\n");

    /* 1.0 Dynamic allocating device number (cat /proc/devices) */
    if (alloc_chrdev_region(&mdev.dev_num, 0, 1, "m_cdev") < 0)
    {
        pr_err("MajorMinorExample.ko > Failed to alloc chrdev region\n");
        return -1;
    }

    // 1.1. static allocating device number (cat /proc/devices)
    // dev_t dev = MKDEV(173, 0);
    // register_chrdev_region(&mdev.dev_num, 1, "m-cdev");

    pr_info("MajorMinorExample.ko > Major = %d, Minor = %d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

    /* 1.0 Dynamic allocating device number (cat /proc/devices) */
    return 0;
}

/* Destructor */
static void __exit chdev_exit(void)
{
    pr_info("MajorMinorExample.ko > Goodbye!\n");
}

module_init(chdev_init);
module_exit(chdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERS);