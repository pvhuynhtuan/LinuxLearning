/*
* sudo dmesg -W                 ==> Show the kernel log
* sudo insmod <.ko file>        ==> install the driver
* sudo rmmod <.ko file>         ==> Remove the driver
*/

#include <linux/module.h> /* Define module_init(), module_exit() */
#include <linux/fs.h> /* define alloc_chrdev_region(), register_chrdev_region() */
#include <linux/device.h> /* Define device_create(), class_create() */

#define DRIVER_AUTHOR       "huynhtuan pvhuynhtuan@gmail.com"
#define DRIVER_DESC         "This is the example for driver module - Major/minor number"
#define DRIVER_VERS         "1.0"

#define DRIVER_DEVICE_NUM_NAME  "my_HT_cdev"
#define DRIVER_DEVICE_NAME  "my_HT_device"

struct m_foo_dev {
    dev_t dev_num;
    struct class *m_class;
} mdev;

/* Constructor */
static int __init chdev_init(void)
{
    pr_info("DeviceFile.ko > Hello world kernel module!\n");

    /* 1.0 Dynamic allocating device number (cat /proc/devices) */
    if (alloc_chrdev_region(&mdev.dev_num, 0, 1, DRIVER_DEVICE_NUM_NAME) < 0)
    {
        pr_err("DeviceFile.ko > Failed to alloc chrdev region\n");
        return -1;
    }

    // 1.1. static allocating device number (cat /proc/devices)
    // dev_t dev = MKDEV(173, 0);
    // register_chrdev_region(&mdev.dev_num, 1, DRIVER_DEVICE_NUM_NAME);

    pr_info("DeviceFile.ko > Major = %d, Minor = %d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

    /* 2.0. Create Struct Class */
    //if ((mdev.m_class = class_create(THIS_MODULE, "m_class")) == NULL)
    if ((mdev.m_class = class_create("m_class")) == NULL) // new kernel version
    {
        pr_err("DeviceFile.ko > Cannot create the struct class for my device\n");
        goto rm_class;
    }

    /* 3.0. Creating device */
    if ((device_create(mdev.m_class, NULL, mdev.dev_num, NULL, DRIVER_DEVICE_NAME)) == NULL)
    {
        pr_err("DeviceFile.ko > Cannot create my device\n");
        goto rm_device_numb;
    }

    pr_info("DeviceFile.ko > End init!\n");
    return 0;

rm_class:
    class_destroy(mdev.m_class);
rm_device_numb:
    unregister_chrdev_region(mdev.dev_num, 1);
    return -1;
}

/* Destructor */
static void __exit chdev_exit(void)
{
    device_destroy(mdev.m_class, mdev.dev_num);
    class_destroy(mdev.m_class);
    unregister_chrdev_region(mdev.dev_num, 1);
    pr_info("DeviceFile.ko > Goodbye!\n");
}

module_init(chdev_init);
module_exit(chdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERS);