/*
* sudo dmesg -W                 ==> Show the kernel log
* sudo insmod <.ko file>        ==> install the driver
* sudo rmmod <.ko file>         ==> Remove the driver
*/

#include <linux/module.h> /* Define module_init(), module_exit() */

#define DRIVER_AUTHOR   "huynhtuan pvhuynhtuan@gmail.com"
#define DRIVER_DESC     "This is the example for driver module"
#define DRIVER_VERS     "1.0"

/* Constructor */
static int __init chdev_init(void)
{
    pr_info("CharacterDeviceExample.ko > Hello world kernel module!\n");
    return 0;
}

/* Destructor */
static void __exit chdev_exit(void)
{
    pr_info("CharacterDeviceExample.ko > Goodbye!\n");
}

module_init(chdev_init);
module_exit(chdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERS);