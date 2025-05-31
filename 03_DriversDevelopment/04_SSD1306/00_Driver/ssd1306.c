/*
* sudo dmesg -w                 ==> Show the kernel log
* sudo insmod <.ko file>        ==> install the driver
* sudo rmmod <.ko file>         ==> Remove the driver
* modprobe                      ==> load driver and dependancy (move ko file to /lib/module/$(uname -r)/extra )
* sudo depmod                   ==> set the dependancy
*/

#include <linux/module.h> /* Define module_init(), module_exit() */
#include <linux/delay.h>
#include <linux/fs.h> /* define alloc_chrdev_region(), register_chrdev_region() */
#include <linux/device.h> /* Define device_create(), class_create() */
#include <linux/cdev.h> /* Define cdev_init(), cdev_add() */
// #include <linux/platform_device.h> /* For platform devices */
// #include <linux/gpio/consumer.h> /* For GPIO descriptor */
// #include <linux/of.h> /* for DT */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>

#define DRIVER_AUTHOR       "huynhtuan pvhuynhtuan@gmail.com"
#define DRIVER_DESC         "This is the example for driver module - ssd1306"
#define DRIVER_VERS         "1.0"

#define DRIVER_DEVICE_NUM_NAME  "ssd1306_cdev"
#define DRIVER_DEVICE_NAME  "ssd1306"

/******** For I2C *********/
#define SSD1306_MAX_SEG         128
#define SSD1306_MAX_LINE        7
#define SSD1306_DEF_FONT_SIZE   5
#define MAX_BUFF                256

static const unsigned char ssd1306_font[][SSD1306_DEF_FONT_SIZE] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x00, 0x2f, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14}, // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1c, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1c, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x00, 0xA0, 0x60, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x59, 0x51, 0x3E}, // @
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x55, 0xAA, 0x55, 0xAA, 0x55}, // Backslash (Checker pattern)
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x03, 0x05, 0x00, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x18, 0xA4, 0xA4, 0xA4, 0x7C}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x40, 0x80, 0x84, 0x7D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0xFC, 0x24, 0x24, 0x24, 0x18}, // p
    {0x18, 0x24, 0x24, 0x18, 0xFC}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x1C, 0xA0, 0xA0, 0xA0, 0x7C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    {0x00, 0x10, 0x7C, 0x82, 0x00}, // {
    {0x00, 0x00, 0xFF, 0x00, 0x00}, // |
    {0x00, 0x82, 0x7C, 0x10, 0x00}, // }
    {0x00, 0x06, 0x09, 0x09, 0x06}  // ~ (Degrees)
};

typedef struct ssd1306_i2c_module {
    struct i2c_client *client;
    uint8_t current_line;
    uint8_t cursor_pos;
    uint8_t font_size;
    struct device *device;

    // For device character file
    dev_t dev_num;
    struct class *ssd1306_class;
    struct cdev ssd1306_cdev;
} ssd1306_i2c_module_t;

char message[MAX_BUFF];
ssd1306_i2c_module_t* module_ssd1306 = NULL;
/**************************/

/********************************************************
 *          Function Declaration                        *
*********************************************************/
static int ssd1306_open(struct inode *inode, struct file *file);
static int ssd1306_release(struct inode *inode, struct file *file);
static ssize_t ssd1306_read(struct file *filp, char __user *user_buf, size_t size, loff_t * offset);
static ssize_t ssd1306_writefd(struct file *filp, const char *user_buf, size_t size, loff_t * offset);

static int ssd1306_i2c_probe(struct i2c_client *client);
static void ssd1306_i2c_remove(struct i2c_client *client);

// The function to control the OLED SSD1307
static int ssd1306_DisplayInit(ssd1306_i2c_module_t *module);
static int ssd1306_I2cWrite(ssd1306_i2c_module_t *module, unsigned char *buf, unsigned int len);
static int ssd1306_I2cRead(ssd1306_i2c_module_t *module, unsigned char *out_buf, unsigned int len);
static void ssd1306_Write(ssd1306_i2c_module_t *module, bool is_cmd, unsigned char data);
static void ssd1306_SetBrightness(ssd1306_i2c_module_t *module, uint8_t brightness);
static void ssd1306_Clear(ssd1306_i2c_module_t *module);
static void ssd1306_SetCursor(ssd1306_i2c_module_t *module, uint8_t current_line, uint8_t cursor_pos);
static void ssd1306_GotoNextLine(ssd1306_i2c_module_t *module);
static void ssd1306_PrintChar(ssd1306_i2c_module_t *module, unsigned char c);
static void ssd1306_PrintString(ssd1306_i2c_module_t *module, unsigned char *str);
static void ssd1306_PrintString(ssd1306_i2c_module_t *module, unsigned char *str);


static const struct of_device_id ssd1306_of_match_ids[] = {
    { .compatible = "ssd1306", 0 },
    { }  /* sentinel */
};

static const struct i2c_device_id ssd1306_i2c_match_ids[] = {
    { "ssd1306", 0 },
    { }
};

static struct i2c_driver ssd1306_i2c_driver = {
    .driver = {
        .name = "ssd1306",
        .owner = THIS_MODULE,
        .of_match_table = ssd1306_of_match_ids,
    },
    .probe = ssd1306_i2c_probe,
    .remove = ssd1306_i2c_remove,
    .id_table = ssd1306_i2c_match_ids,
};

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = ssd1306_read,
    .write      = ssd1306_writefd,
    .open       = ssd1306_open,
    .release    = ssd1306_release,
};
/*****************************************************************************************
 * The function to handle the device file character
 ****************************************************************************************/
/* Create device character file */
static int ssd1306_CreateDeviceFile(ssd1306_i2c_module_t *module)
{
    pr_info("ssd1306.ko > Hello ssd1306_create_device_file!\n");

    /* 1.0 Dynamic allocating device number (cat /proc/devices) */
    if (alloc_chrdev_region(&module->dev_num, 0, 1, DRIVER_DEVICE_NUM_NAME) < 0)
    {
        pr_err("ssd1306.ko > Failed to alloc chrdev region\n");
        return -1;
    }

    // 1.1. static allocating device number (cat /proc/devices)
    // dev_t dev = MKDEV(173, 0);
    // register_chrdev_region(&mdev.dev_num, 1, DRIVER_DEVICE_NUM_NAME);

    pr_info("ssd1306.ko > Major = %d, Minor = %d\n", MAJOR(module->dev_num), MINOR(module->dev_num));

    /* 2.0. Create Struct Class */
    if ((module->ssd1306_class = class_create("ssd1306_class")) == NULL) // new kernel version
    {
        pr_err("ssd1306.ko > Cannot create the struct class for my device\n");
        goto rm_class;
    }

    /* 3.0. Creating device */
    if ((module->device = device_create(module->ssd1306_class, NULL, module->dev_num, NULL, DRIVER_DEVICE_NAME)) == NULL)
    {
        pr_err("ssd1306.ko > Cannot create my device\n");
        goto rm_device_numb;
    }

    /* 4.0. Create cdev structure */
    cdev_init(&module->ssd1306_cdev, &fops);

    /* 4.1. Adding character devide to system */
    if ((cdev_add(&module->ssd1306_cdev, module->dev_num, 1)) < 0)
    {
        pr_err("ssd1306.ko > Cannot add the device to system\n");
        goto rm_device;
    }

    pr_info("ssd1306.ko > End create device file!\n");
    return 0;

rm_device:
    device_destroy(module->ssd1306_class, module->dev_num);
rm_class:
    class_destroy(module->ssd1306_class);
rm_device_numb:
    unregister_chrdev_region(module->dev_num, 1);
    return -1;
}

// This function will be called when we open the Device File
static int ssd1306_open(struct inode *inode, struct file *file)
{
    pr_info("ssd1306.ko > System Call open() was called!\n");
    return 0;
}

// This function will be called when we close the Device File
static int ssd1306_release(struct inode *inode, struct file *file)
{
    pr_info("ssd1306.ko > System Call close() was called!\n");
    return 0;
}

// Read API
static ssize_t ssd1306_read(struct file *filp, char __user *user_buf, size_t size, loff_t * offset)
{
    pr_info("ssd1306.ko > System Call read() called!\n");
    return 1;
}

// Write API
static ssize_t ssd1306_writefd(struct file *filp, const char *user_buf, size_t size, loff_t * offset)
{
    pr_info("ssd1306.ko > System Call write() called!\n");

    /* Copy from user buffer to mapped area */
    // memset(&mdev.level, 0, 1);
    // if (copy_from_user(&mdev.level, user_buf, size) != 0)
    // {
    //     return -EFAULT;
    // }

    // pr_info("ssd1306.ko > Data from user: %d\n", mdev.level);

    return 1;
}

/*****************************************************************************************
 * The function to handle the device matching
 ****************************************************************************************/
static int ssd1306_i2c_probe(struct i2c_client *client)
{
    ssd1306_i2c_module_t *module;

    module = kmalloc(sizeof(*module), GFP_KERNEL);
    if (!module) {
        pr_err("ssd1306.ko > kmalloc failed!\n");
        return -1;
    }

    module->client = client;
    module->current_line = 0;
    module->cursor_pos = 0;
    module->font_size = SSD1306_DEF_FONT_SIZE;
    i2c_set_clientdata(client, module);

    ssd1306_DisplayInit(module);
    ssd1306_SetCursor(module, 0, 0);
    ssd1306_PrintString(module, "Hello Tuan!");
    
    if (ssd1306_CreateDeviceFile(module) != 0) {
        kfree(module);
        pr_err("ssd1306.ko > create device file failed\n");
        return -1;
    }
    
    module_ssd1306 = module;
    pr_info("ssd1306.ko > Probe success!\n");

    return 0;
}

static void ssd1306_i2c_remove(struct i2c_client *client)
{
    ssd1306_i2c_module_t *module = i2c_get_clientdata(client);

    ssd1306_PrintString(module, "Removing the device!!!");

    cdev_del(&module->ssd1306_cdev);
    device_destroy(module->ssd1306_class, module->dev_num);
    class_destroy(module->ssd1306_class);
    unregister_chrdev_region(module->dev_num, 1);
    
    msleep(1000);
    ssd1306_Clear(module);
    ssd1306_Write(module, true, 0xAE); // Entire Display OFF
    pr_info("ssd1306.ko > driver remove!\n");

    kfree(module);
}

// Matching the device tree
MODULE_DEVICE_TABLE(of, ssd1306_of_match_ids);
MODULE_DEVICE_TABLE(i2c, ssd1306_i2c_match_ids);  // Correct for I2C legacy matching
module_i2c_driver(ssd1306_i2c_driver);

/*****************************************************************************************
 * The macro to register module information
 ****************************************************************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERS);

/*****************************************************************************************
 * The function to control the OLED SSD1307
 ****************************************************************************************/
static int ssd1306_DisplayInit(ssd1306_i2c_module_t *module)
{
    msleep(100);
    ssd1306_Write(module, true, 0xAE); // Entire Display OFF
    ssd1306_Write(module, true, 0xD5); // Set Display Clock Divide Ratio and Oscillator Frequency
    ssd1306_Write(module, true, 0x80); // Default Setting for Display Clock Divide Ratio and Oscillator Frequency that is recommended
    ssd1306_Write(module, true, 0xA8); // Set Multiplex Ratio
    ssd1306_Write(module, true, 0x3F); // 64 COM lines
    ssd1306_Write(module, true, 0xD3); // Set display offset
    ssd1306_Write(module, true, 0x00); // 0 offset
    ssd1306_Write(module, true, 0x40); // Set first line as the start line of the display
    ssd1306_Write(module, true, 0x8D); // Charge pump
    ssd1306_Write(module, true, 0x14); // Enable charge dump during display on
    ssd1306_Write(module, true, 0x20); // Set memory addretsing mode
    ssd1306_Write(module, true, 0x00); // Horizontal addretsing mode
    ssd1306_Write(module, true, 0xA1); // Set segment remap with column addrets 127 mapped to segment 0
    ssd1306_Write(module, true, 0xC8); // Set com output scan direction, scan from com63 to com 0
    ssd1306_Write(module, true, 0xDA); // Set com pins hardware configuration
    ssd1306_Write(module, true, 0x12); // Alternative com pin configuration, disable com left/right remap
    ssd1306_Write(module, true, 0x81); // Set contrast control
    ssd1306_Write(module, true, 0x80); // Set Contrast to 128
    ssd1306_Write(module, true, 0xD9); // Set pre-charge period
    ssd1306_Write(module, true, 0xF1); // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK
    ssd1306_Write(module, true, 0xDB); // Set Vcomh deselect level
    ssd1306_Write(module, true, 0x20); // Vcomh deselect level ~ 0.77 Vcc
    ssd1306_Write(module, true, 0xA4); // Entire display ON, retume to RAM content display
    ssd1306_Write(module, true, 0xA6); // Set Display in Normal Mode, 1 = ON, 0 = OFF
    ssd1306_Write(module, true, 0x2E); // Deactivate scroll
    ssd1306_Write(module, true, 0xAF); // Display ON in normal mode
    ssd1306_Clear(module);

    return 0;
}

static int ssd1306_I2cWrite(ssd1306_i2c_module_t *module, unsigned char *buf, unsigned int len)
{
    int ret = i2c_master_send(module->client, buf, len);
    if (ret < 0) {
        pr_err("ssd1306.ko > [%s - %d] Failed to send data over I2C: %d\n", __func__, __LINE__, ret);
    }
    return ret;
}

static int ssd1306_I2cRead(ssd1306_i2c_module_t *module, unsigned char *out_buf, unsigned int len)
{
    int ret = i2c_master_recv(module->client, out_buf, len);
    if (ret < 0) {
        pr_err("ssd1306.ko > [%s - %d] Failed to recive data from I2C: %d\n", __func__, __LINE__, ret);
    }
    return ret;
}

static void ssd1306_Write(ssd1306_i2c_module_t *module, bool is_cmd, unsigned char data)
{
    unsigned char buf[2] = {0};

    if (is_cmd == true) {
        buf[0] = 0x00;
    } else {
        buf[0] = 0x40;
    }

    buf[1] = data;
    ssd1306_I2cWrite(module, buf, 2);
}

static void ssd1306_SetBrightness(ssd1306_i2c_module_t *module, uint8_t brightness)
{
    ssd1306_Write(module, true, 0x81);
    ssd1306_Write(module, true, brightness);
}

static void ssd1306_Clear(ssd1306_i2c_module_t *module)
{
    unsigned int total = 128 * 8;
    int i = 0;

    for (i = 0; i < total; i++) {
        ssd1306_Write(module, false, 0);
    }
}

static void ssd1306_SetCursor(ssd1306_i2c_module_t *module, uint8_t current_line, uint8_t cursor_pos)
{
    if ((current_line <= SSD1306_MAX_LINE) && (cursor_pos < SSD1306_MAX_SEG)) {
        module->current_line = current_line;    // Save the specified line number
        module->cursor_pos = cursor_pos;        // Save the specified cursor position
        ssd1306_Write(module, true, 0x21);                  // cmd for the column start and end addrets
        ssd1306_Write(module, true, cursor_pos);            // column start addr
        ssd1306_Write(module, true, SSD1306_MAX_SEG - 1);   // column end addr
        ssd1306_Write(module, true, 0x22);                  // cmd for the page start and end addrets
        ssd1306_Write(module, true, current_line);          // page start addr
        ssd1306_Write(module, true, SSD1306_MAX_LINE);      // page end addr
    }
}

static void ssd1306_GotoNextLine(ssd1306_i2c_module_t *module)
{
        module->current_line++;
        module->current_line = (module->current_line & SSD1306_MAX_LINE);
        ssd1306_SetCursor(module, module->current_line, 0);
}

static void ssd1306_PrintChar(ssd1306_i2c_module_t *module, unsigned char c)
{
    uint8_t data_byte;
    uint8_t temp = 0;

    if (((module->cursor_pos + module->font_size) >= SSD1306_MAX_SEG) || (c == '\n')) {
        ssd1306_GotoNextLine(module);
    }

    if (c != '\n') {
        c -= 0x20;
        do {
            data_byte = ssd1306_font[c][temp];
            ssd1306_Write(module, false, data_byte);
            module->cursor_pos++;
            temp++;
        } while (temp < module->font_size);

        ssd1306_Write(module, false, 0x00);
        module->cursor_pos++;
    }
}

static void ssd1306_PrintString(ssd1306_i2c_module_t *module, unsigned char *str)
{
    ssd1306_Clear(module);
    ssd1306_SetCursor(module, 0, 0);

    while (*str) {
        ssd1306_PrintChar(module, *str++);
    }
}
