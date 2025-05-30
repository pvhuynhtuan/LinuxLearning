/*
* sudo dmesg -w                 ==> Show the kernel log
* sudo insmod <.ko file>        ==> install the driver
* sudo rmmod <.ko file>         ==> Remove the driver
* modprobe                      ==> load driver and dependancy (move ko file to /lib/module/$(uname -r)/extra )
* sudo depmod                   ==> set the dependancy
*/

#include <linux/module.h> /* Define module_init(), module_exit() */
#include <linux/gpio.h>
#include <linux/fs.h> /* define alloc_chrdev_region(), register_chrdev_region() */
#include <linux/device.h> /* Define device_create(), class_create() */
#include <linux/cdev.h> /* Define cdev_init(), cdev_add() */
#include <linux/platform_device.h> /* For platform devices */
#include <linux/gpio/consumer.h> /* For GPIO descriptor */
#include <linux/of.h> /* for DT */

#define DRIVER_AUTHOR       "huynhtuan pvhuynhtuan@gmail.com"
#define DRIVER_DESC         "This is the example for driver module - GPIOSubSystemDriver - Interger Based"
#define DRIVER_VERS         "1.0"

#define DRIVER_DEVICE_NUM_NAME  "GPIOSubSystemDriver_cdev"
#define DRIVER_DEVICE_NAME  "GPIOSubSystemDriver"

/******** For GPIO *********/
//#define DRIVER_GPIO_NUMBER      539  // Adjusted for base 512
#define DRIVER_GPIO_HIGH        1
#define DRIVER_GPIO_LOW         0

struct gpio_desc *gpio_27;

static const struct of_device_id gpiod_dt_ids[] =
{
    { .compatible = "gpio-descriptor-based", },
    { /* sentinel */ }
};
/**************************/

struct m_foo_dev {
    char level;
    dev_t dev_num;
    struct class *m_class;
    struct cdev m_cdev;
} mdev;

/* Function prototypes */
static int __init chdev_init(void);
static void __exit chdev_exit(void);
static int m_open(struct inode *inode, struct file *file);
static int m_release(struct inode *inode, struct file *file);
static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t * offset);
static ssize_t m_write(struct file *filp, const char *user_buf, size_t size, loff_t * offset);

static int gpiosubsystem_driver_probe(struct platform_device *pdev);
static void gpiosubsystem_driver_remove(struct platform_device *pdev);

static struct platform_driver gpiosubsystem =
{
    .probe = gpiosubsystem_driver_probe,
    .remove = gpiosubsystem_driver_remove,
    .driver = {
        .name = "gpio-descriptor-based",
        .of_match_table = of_match_ptr(gpiod_dt_ids),
        .owner = THIS_MODULE,
    },
};

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = m_read,
    .write      = m_write,
    .open       = m_open,
    .release    = m_release,
};

/********************************************************
 *          Function Declaration                        *
*********************************************************/
/* Constructor */
static int __init chdev_init(void)
{
    pr_info("GPIOSubSystemDriver.ko > Hello world kernel module!\n");

    /* 1.0 Dynamic allocating device number (cat /proc/devices) */
    if (alloc_chrdev_region(&mdev.dev_num, 0, 1, DRIVER_DEVICE_NUM_NAME) < 0)
    {
        pr_err("GPIOSubSystemDriver.ko > Failed to alloc chrdev region\n");
        return -1;
    }

    // 1.1. static allocating device number (cat /proc/devices)
    // dev_t dev = MKDEV(173, 0);
    // register_chrdev_region(&mdev.dev_num, 1, DRIVER_DEVICE_NUM_NAME);

    pr_info("GPIOSubSystemDriver.ko > Major = %d, Minor = %d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

    /* 2.0. Create Struct Class */
    //if ((mdev.m_class = class_create(THIS_MODULE, "m_class")) == NULL)
    if ((mdev.m_class = class_create("m_class")) == NULL) // new kernel version
    {
        pr_err("GPIOSubSystemDriver.ko > Cannot create the struct class for my device\n");
        goto rm_class;
    }

    /* 3.0. Creating device */
    if ((device_create(mdev.m_class, NULL, mdev.dev_num, NULL, DRIVER_DEVICE_NAME)) == NULL)
    {
        pr_err("GPIOSubSystemDriver.ko > Cannot create my device\n");
        goto rm_device_numb;
    }

    /* 4.0. Create cdev structure */
    cdev_init(&mdev.m_cdev, &fops);

    /* 4.1. Adding character devide to system */
    if ((cdev_add(&mdev.m_cdev, mdev.dev_num, 1)) < 0)
    {
        pr_err("GPIOSubSystemDriver.ko > Cannot add the device to system\n");
        goto rm_device;
    }

    /* USER CODE */
    if (platform_driver_register(&gpiosubsystem))
    {
        pr_err("GPIOSubSystemDriver.ko > Failed to register platform driver\n");
        return -ENODEV;
    }

    pr_info("GPIOSubSystemDriver.ko > End init!\n");
    return 0;

rm_device:
    device_destroy(mdev.m_class, mdev.dev_num);
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
    // Unregister platform driver
    platform_driver_unregister(&gpiosubsystem);
    pr_info("GPIOSubSystemDriver.ko > Goodbye!\n");
}

// This function will be called when we open the Device File
static int m_open(struct inode *inode, struct file *file)
{
    pr_info("GPIOSubSystemDriver.ko > System Call open() was called!\n");
    return 0;
}

// This function will be called when we close the Device File
static int m_release(struct inode *inode, struct file *file)
{
    pr_info("GPIOSubSystemDriver.ko > System Call close() was called!\n");
    return 0;
}

// Read API
static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t * offset)
{
    pr_info("GPIOSubSystemDriver.ko > System Call read() called!\n");
    return 1;
}

// Write API
static ssize_t m_write(struct file *filp, const char *user_buf, size_t size, loff_t * offset)
{
    pr_info("GPIOSubSystemDriver.ko > System Call write() called!\n");

    /* Copy from user buffer to mapped area */
    memset(&mdev.level, 0, 1);
    if (copy_from_user(&mdev.level, user_buf, size) != 0)
    {
        return -EFAULT;
    }

    pr_info("GPIOSubSystemDriver.ko > Data from user: %d\n", mdev.level);

    if (mdev.level == 1)
    {
        pr_info("GPIOSubSystemDriver.ko > LED on\n");
        gpiod_set_value(gpio_27, DRIVER_GPIO_HIGH);
    }
    else
    {
        pr_info("GPIOSubSystemDriver.ko > LED off\n");
        gpiod_set_value(gpio_27, DRIVER_GPIO_LOW);
    }
    

    return 1;
}

static int gpiosubsystem_driver_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev-> dev;
    pr_info("GPIOSubSystemDriver.ko > driver probe!\n");

    // set gpio 27 to high
    gpio_27 = gpiod_get(dev, "led27", GPIOD_OUT_LOW);
    gpiod_set_value(gpio_27, DRIVER_GPIO_HIGH);

    return 0;
}

static void gpiosubsystem_driver_remove(struct platform_device *pdev)
{
    pr_info("GPIOSubSystemDriver.ko > driver remove!\n");

    gpiod_set_value(gpio_27, DRIVER_GPIO_LOW);
    gpiod_put(gpio_27);
}

module_init(chdev_init);
module_exit(chdev_exit);

//module_platform_driver(gpiosubsystem);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERS);