/**
 * @file ili9341.c
 * @brief SPI Device Driver for ILI9341 240x320 LCD display
 * 
 * This driver implements a frame buffer interface to control
 * a ILI9341 LCD display connected via SPI to a Raspberry Pi.
 */

/*
* Command to quick install the driver
* sudo dmesg -w                 ==> Show the kernel log
* sudo insmod <.ko file>        ==> install the driver
* sudo rmmod <.ko file>         ==> Remove the driver
* modprobe                      ==> load driver and dependancy (move ko file to /lib/module/$(uname -r)/extra )
* sudo depmod                   ==> set the dependancy
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h> /* Define module_init(), module_exit() */
#include <linux/fs.h> /* define alloc_chrdev_region(), register_chrdev_region() */

#include <linux/device.h> /* Define device_create(), class_create() */
#include <linux/cdev.h> /* Define cdev_init(), cdev_add() */

#include <linux/fb.h>   /* for frame buffer */
#include <linux/dma-mapping.h> /* For DMA */
#include <linux/dmaengine.h>

#include <linux/delay.h>
#include <linux/kthread.h>
// For hardware ip
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <linux/spi/spi.h>
#include <linux/of_gpio.h>

#define DRIVER_AUTHOR       "huynhtuan pvhuynhtuan@gmail.com"
#define DRIVER_DESC         "This is the driver module to control the LCD ILI9341"
#define DRIVER_VERS         "1.0"

/*****************************************************************************
 *                       DRIVER CONFIGURATION                                *
*****************************************************************************/
#define ILI9341_ON              1
#define ILI9341_OFF             0

#define ILI9341_CDEV_CREATION   ILI9341_OFF
#define ILI9341_FB_CREATION     ILI9341_ON

#define ILI9341_FB_DMA_ENABLE   ILI9341_ON
#define ILI9341_FB_IMAGEBLIT_ENABLE ILI9341_ON


/*****************************************************************************
 *                          DEFINE SECTION                                   *
*****************************************************************************/
#if (ILI9341_CDEV_CREATION == ILI9341_ON) 
#define DRIVER_DEVICE_NUM_NAME  "ili9341_cdev"
#define DRIVER_DEVICE_NAME  "ili9341"
#endif

#if (ILI9341_FB_CREATION == ILI9341_ON)
#define DRIVER_FB_NAME  "ili9341_fb"
#endif

/******** For SPI *********/
/* GPIO states */
#define GPIO_LOW                0
#define GPIO_HIGH               1

/* SPI Speed */
#define ILI9341_SPI_SPEED       8000000

/* LCD modes */
#define ILI9341_MODE_CMD        0
#define ILI9341_MODE_DATA       1

#define ILI9341_TFTWIDTH        320 ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT       240 ///< ILI9341 max TFT height
#define ILI9341_BIT_PER_PIXEL   16

#define ILI9341_FB_SIZE_RAW     ((ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT * ILI9341_BIT_PER_PIXEL) / 8) // = 153600
#define ILI9341_BUFFER_SIZE     PAGE_ALIGN(ILI9341_FB_SIZE_RAW) // = 155648

/******************************************************************************
** ILI9341 COMMAND LIST                                                       *
******************************************************************************/
#define ILI9341_NOP             0x00 ///< No-op register
#define ILI9341_SWRESET         0x01 ///< Software reset register
#define ILI9341_RDDID           0x04 ///< Read display identification information
#define ILI9341_RDDST           0x09 ///< Read Display Status

#define ILI9341_SLPIN           0x10 ///< Enter Sleep Mode
#define ILI9341_SLPOUT          0x11 ///< Sleep Out
#define ILI9341_PTLON           0x12 ///< Partial Mode ON
#define ILI9341_NORON           0x13 ///< Normal Display Mode ON

#define ILI9341_RDMODE          0x0A ///< Read Display Power Mode
#define ILI9341_RDMADCTL        0x0B ///< Read Display MADCTL
#define ILI9341_RDPIXFMT        0x0C ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT        0x0D ///< Read Display Image Format
#define ILI9341_RDSELFDIAG      0x0F ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF          0x20 ///< Display Inversion OFF
#define ILI9341_INVON           0x21 ///< Display Inversion ON
#define ILI9341_GAMMASET        0x26 ///< Gamma Set
#define ILI9341_DISPOFF         0x28 ///< Display OFF
#define ILI9341_DISPON          0x29 ///< Display ON

#define ILI9341_CASET           0x2A ///< Column Address Set
#define ILI9341_PASET           0x2B ///< Page Address Set
#define ILI9341_RAMWR           0x2C ///< Memory Write
#define ILI9341_RAMRD           0x2E ///< Memory Read

#define ILI9341_PTLAR           0x30 ///< Partial Area
#define ILI9341_VSCRDEF         0x33 ///< Vertical Scrolling Definition
#define ILI9341_MADCTL          0x36 ///< Memory Access Control
#define ILI9341_VSCRSADD        0x37 ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT          0x3A ///< COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1         0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2         0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3         0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR          0xB4  ///< Display Inversion Control
#define ILI9341_DFUNCTR         0xB6 ///< Display Function Control

#define ILI9341_PWCTR1          0xC0 ///< Power Control 1
#define ILI9341_PWCTR2          0xC1 ///< Power Control 2
#define ILI9341_PWCTR3          0xC2 ///< Power Control 3
#define ILI9341_PWCTR4          0xC3 ///< Power Control 4
#define ILI9341_PWCTR5          0xC4 ///< Power Control 5
#define ILI9341_VMCTR1          0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2          0xC7 ///< VCOM Control 2

#define ILI9341_RDID1           0xDA ///< Read ID 1
#define ILI9341_RDID2           0xDB ///< Read ID 2
#define ILI9341_RDID3           0xDC ///< Read ID 3
#define ILI9341_RDID4           0xDD ///< Read ID 4

#define ILI9341_GMCTRP1         0xE0 ///< Positive Gamma Correction
#define ILI9341_GMCTRN1         0xE1 ///< Negative Gamma Correction
#define ILI9341_PWCTR6          0xFC

// Color definitions
#define ILI9341_BLACK           0x0000 ///<   0,   0,   0
#define ILI9341_NAVY            0x000F ///<   0,   0, 123
#define ILI9341_DARKGREEN       0x03E0 ///<   0, 125,   0
#define ILI9341_DARKCYAN        0x03EF ///<   0, 125, 123
#define ILI9341_MAROON          0x7800 ///< 123,   0,   0
#define ILI9341_PURPLE          0x780F ///< 123,   0, 123
#define ILI9341_OLIVE           0x7BE0 ///< 123, 125,   0
#define ILI9341_LIGHTGREY       0xC618 ///< 198, 195, 198
#define ILI9341_DARKGREY        0x7BEF ///< 123, 125, 123
#define ILI9341_BLUE            0x001F ///<   0,   0, 255
#define ILI9341_GREEN           0x07E0 ///<   0, 255,   0
#define ILI9341_CYAN            0x07FF ///<   0, 255, 255
#define ILI9341_RED             0xF800 ///< 255,   0,   0
#define ILI9341_MAGENTA         0xF81F ///< 255,   0, 255
#define ILI9341_YELLOW          0xFFE0 ///< 255, 255,   0
#define ILI9341_WHITE           0xFFFF ///< 255, 255, 255
#define ILI9341_ORANGE          0xFD20 ///< 255, 165,   0
#define ILI9341_GREENYELLOW     0xAFE5 ///< 173, 255,  41
#define ILI9341_PINK            0xFC18 ///< 255, 130, 198
/*********************   END OF ILI9341 COMMAND LIST   ***********************/

typedef struct ILI9341_SPIModule {
    struct spi_device *pSpiDev;

    /* GPIO pins */
    int iRstPin;
    int iDcPin;

    #if (ILI9341_CDEV_CREATION == ILI9341_ON)
    /* for device */
    struct device *pDevice;
    
    // For device character file
    dev_t sDevNum;
    struct class *pClass;
    struct cdev pCdev;
    #endif

    #if (ILI9341_FB_CREATION == ILI9341_ON)
    struct fb_info *pFbInfo;
    #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
    dma_addr_t sDmaHandle;        // DMA bus address
    #endif
    #endif
} ILI9341_SPIModule_t;

ILI9341_SPIModule_t* gpModuleILI9341 = NULL;
/**************************/

/********************************************************
 *          Function Declaration                        *
*********************************************************/
#if (ILI9341_CDEV_CREATION == ILI9341_ON)
static int ILI9341_CreateDeviceFile(ILI9341_SPIModule_t *lpModule);
static int ILI9341_CdevOpen(struct inode *inode, struct file *file);
static int ILI9341_CdevRelease(struct inode *inode, struct file *file);
static ssize_t ILI9341_CdevRead(struct file *filp, char __user *user_buf, size_t size, loff_t * offset);
static ssize_t ILI9341_CdevWrite(struct file *filp, const char *user_buf, size_t size, loff_t * offset);
#endif

#if (ILI9341_FB_CREATION == ILI9341_ON)
static int ILI9341_CreateFbFile(ILI9341_SPIModule_t *lpModule);
static int ILI9341_FbOpen(struct fb_info *lpInfo, int liUser);
static int ILI9341_FbRelease(struct fb_info *lpInfo, int liUser);
static ssize_t ILI9341_FbWrite(struct fb_info *info, const char __user *buf, size_t count, loff_t *ppos);
static int ILI9341_FbCheckVar(struct fb_var_screeninfo *var, struct fb_info *info);
static int ILI9341_FbSetPar(struct fb_info *info);
static int ILI9341_FbSetColReg(unsigned regno, unsigned red, unsigned green, unsigned blue, unsigned transp, struct fb_info *info);
static int ILI9341_FbMmap(struct fb_info *info, struct vm_area_struct *vma);
static void ILI9341_FbFillRect(struct fb_info *info, const struct fb_fillrect *rect);
static void ILI9341_FbCopyArea(struct fb_info *info, const struct fb_copyarea *region);
#if (ILI9341_FB_IMAGEBLIT_ENABLE == ILI9341_ON)
static void ILI9341_FbImageBlit(struct fb_info *info, const struct fb_image *image);
#endif
static int ILI9341_FbBlank(int blank_mode, struct fb_info *info);
#if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
static void ILI9341_DeferredIo(struct fb_info *info, struct list_head *pagelist);
static void ILI9341_FbDmaFlush(ILI9341_SPIModule_t *lpModule);
#endif
#endif

static int ILI9341_SpiProbe(struct spi_device *lpSpi);
static void ILI9341_SpiRemove(struct spi_device *lpSpi);

// The function to control the LCD
#if (ILI9341_FB_CREATION == ILI9341_ON)
#if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
static int ILI9341_SpiWriteDma(void *lpBuf, size_t liLen);
#endif
#endif
static int ILI9341_SpiSendByte(bool lbIsData, unsigned char lcData);
static int ILI9341_SpiSendArray(bool lbIsData, unsigned char * lpData, unsigned int liLen);
// static int ILI9341_SpiSendData16(unsigned char * lpData);
// static int ILI9341_SpiSendArray16(unsigned char * lpData, unsigned int liLen);
// static int ILI9341_SpiSendData18(unsigned char * lpData);
// static int ILI9341_SpiSendArray18(unsigned char * lcData, unsigned int liLen);
static int ILI9341_DisplayInit(ILI9341_SPIModule_t *module);
static void ILI9341_DisplayDeinit(ILI9341_SPIModule_t *module);

static const struct of_device_id ili9341_of_match_ids[] = {
    { .compatible = "ili9341", 0 },
    { }  /* sentinel */
};

static const struct spi_device_id ili9341_spi_match_ids[] = {
    { "ili9341", 0 },
    { /* sentinel */ }
};

/* SPI driver structure */
static struct spi_driver gsILI9341SpiDriver = {
    .driver = {
        .name = "ili9341",
        .owner = THIS_MODULE,
        .of_match_table = ili9341_of_match_ids,
    },
    .probe = ILI9341_SpiProbe,
    .remove = ILI9341_SpiRemove,
    .id_table = ili9341_spi_match_ids,
};

#if (ILI9341_CDEV_CREATION == ILI9341_ON)
static struct file_operations gsFops =
{
    .owner      = THIS_MODULE,
    .read       = ILI9341_CdevRead,
    .write      = ILI9341_CdevWrite,
    .open       = ILI9341_CdevOpen,
    .release    = ILI9341_CdevRelease,
};
#endif /* End of #if (ILI9341_CDEV_CREATION == ILI9341_ON) */

#if (ILI9341_FB_CREATION == ILI9341_ON)
/*
* 1. Fill a struct fb_var_screeninfo structure in order to provide information on
* framebuffer variable properties. Those properties can be changed by user space.
*/
static struct fb_var_screeninfo gsFbVarScreenInfo =
{
    .xres = ILI9341_TFTWIDTH, /* visible resolution */
    .yres = ILI9341_TFTHEIGHT,
    .xres_virtual = ILI9341_TFTWIDTH, /* virtual resolution */
    .yres_virtual = ILI9341_TFTHEIGHT,
    .xoffset = 0, /* offset from virtual to visible resolution */
    .yoffset = 0,
    .bits_per_pixel = ILI9341_BIT_PER_PIXEL, /* # of bits needed to hold a pixel */
    // .left_margin, /* time from sync to picture */
    // .right_margin, /* time from picture to sync */
    // .upper_margin, /* time from sync to picture */
    // .lower_margin,
    // .hsync_len, /* length of horizontal sync */
    // .vsync_len, /* length of vertical sync */
    // .rotate, /* angle we rotate counter clockwise */
};

/*
* 2. Fill a struct fb_fix_screeninfo structure, to provide fixed parameters.
*/
static struct fb_fix_screeninfo gsFbFixScreenInfo =
{
    .id = "ILI9341 SCREEN", /* identification string example "TT Builtin" */
    .smem_len = ILI9341_BUFFER_SIZE, /* Length of frame buffer mem */
    .xpanstep = 0, /* zero if no hardware panning */
    .ypanstep = 0, /* zero if no hardware panning */
    .ywrapstep = 0, /* zero if no hardware ywrap */
    .line_length = ILI9341_TFTWIDTH * 3, /* length of a line in bytes */
};

/*
* 3. Set up a struct fb_ops structure, providing necessary callback functions,
* which will used by the framebuffer subsystem in response to user actions.
* 4. Still in the struct fb_ops structure, you have to provide accelerated
* functions callback, if supported by the device.
*/

static struct fb_ops gsFbOps =
{
    .owner = THIS_MODULE,
    .fb_open = ILI9341_FbOpen,
    .fb_release = ILI9341_FbRelease,
    .fb_write = ILI9341_FbWrite,
    .fb_check_var = ILI9341_FbCheckVar, /* checks var and eventually tweaks it to something supported */
    .fb_set_par = ILI9341_FbSetPar, /* set the video mode according to info->var */
    .fb_setcolreg = ILI9341_FbSetColReg, /* set color register */
    .fb_mmap = ILI9341_FbMmap, /* perform fb specific mmap */
    .fb_fillrect = ILI9341_FbFillRect, /* Draws a rectangle */
    .fb_copyarea = ILI9341_FbCopyArea, /* Copy data from area to another */
#if (ILI9341_FB_IMAGEBLIT_ENABLE == ILI9341_ON)
    .fb_imageblit = ILI9341_FbImageBlit, /* Draws a image to the display */
#endif
    .fb_blank = ILI9341_FbBlank, /* blank display */
};

#if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
static struct fb_deferred_io gsILI9341DefIo =
{
    .delay = HZ / 20,               // 50 ms
    .deferred_io = ILI9341_DeferredIo,
};
#endif

#endif /* End of #if (ILI9341_FB_CREATION == ILI9341_ON) */

/*****************************************************************************************
 * The function to handle the device file character
 ****************************************************************************************/
#if (ILI9341_CDEV_CREATION == ILI9341_ON)
 /* Create device character file */
static int ILI9341_CreateDeviceFile(ILI9341_SPIModule_t *lpModule)
{
    pr_info("[%s - %d] > Creating the charater device file!\n", __func__, __LINE__);

    /* 1.0 Dynamic allocating device number (cat /proc/devices) */
    if (alloc_chrdev_region(&lpModule->sDevNum, 0, 1, DRIVER_DEVICE_NUM_NAME) < 0)
    {
        pr_err("[%s - %d] > Failed to alloc chrdev region\n", __func__, __LINE__);
        return -1;
    }
    else
    {
        // Do nothing
    }

    // 1.1. static allocating device number (cat /proc/devices)
    // dev_t dev = MKDEV(173, 0);
    // register_chrdev_region(&mdev.sDevNum, 1, DRIVER_DEVICE_NUM_NAME);

    pr_info("[%s - %d] > Major = %d, Minor = %d\n",
        __func__, __LINE__, MAJOR(lpModule->sDevNum), MINOR(lpModule->sDevNum));

    /* 2.0. Create Struct Class */
    if ((lpModule->pClass = class_create("pClass")) == NULL) // new kernel version
    {
        pr_err("[%s - %d] > Cannot create the struct class for my device\n",
            __func__, __LINE__);
        goto LABEL_RM_CLASS;
    }
    else
    {
        // Do nothing
    }

    /* 3.0. Creating device */
    if ((lpModule->pDevice = device_create(lpModule->pClass, NULL, lpModule->sDevNum, NULL, DRIVER_DEVICE_NAME)) == NULL)
    {
        pr_err("[%s - %d] > Cannot create my device\n", __func__, __LINE__);
        goto LABEL_DEVICE_NUMB;
    }
    else
    {
        // Do nothing
    }

    /* 4.0. Create cdev structure */
    cdev_init(&lpModule->pCdev, &gsFops);

    /* 4.1. Adding character devide to system */
    if ((cdev_add(&lpModule->pCdev, lpModule->sDevNum, 1)) < 0)
    {
        pr_err("[%s - %d] > Cannot add the device to system\n",
            __func__, __LINE__);
        goto LABEL_RM_DEVICE;
    }
    else
    {
        // Do nothing
    }

    pr_info("[%s - %d] > End create device file!\n", __func__, __LINE__);
    return 0;

LABEL_RM_DEVICE:
    device_destroy(lpModule->pClass, lpModule->sDevNum);
LABEL_RM_CLASS:
    class_destroy(lpModule->pClass);
LABEL_DEVICE_NUMB:
    unregister_chrdev_region(lpModule->sDevNum, 1);
    return -1;
}

// This function will be called when we open the Device File
static int ILI9341_CdevOpen(struct inode *inode, struct file *file)
{
    pr_info("[%s - %d] > System Call open() was called!\n", __func__, __LINE__);
    return 0;
}

// This function will be called when we close the Device File
static int ILI9341_CdevRelease(struct inode *inode, struct file *file)
{
    pr_info("[%s - %d] > System Call close() was called!\n", __func__, __LINE__);
    return 0;
}

// Read API
static ssize_t ILI9341_CdevRead(struct file *filp, char __user *user_buf, size_t size, loff_t * offset)
{
    pr_info("[%s - %d] > System Call read() called!\n", __func__, __LINE__);
    return 1;
}

// Write API
static ssize_t ILI9341_CdevWrite(struct file *filp, const char *user_buf, size_t size, loff_t * offset)
{
    pr_info("[%s - %d] > System Call write() called!\n", __func__, __LINE__);
    return 1;
}
#endif /* End of #if (ILI9341_CDEV_CREATION == ILI9341_ON) */

/*****************************************************************************************
 * The function to handle the Frame Buffer
 ****************************************************************************************/
#if (ILI9341_FB_CREATION == ILI9341_ON)
 /* Create Frame Buffer file */
static int ILI9341_CreateFbFile(ILI9341_SPIModule_t *lpModule)
{
    pr_info("[%s - %d] > Creating the Frame Buffer file!\n", __func__, __LINE__);
    
    lpModule->pFbInfo = framebuffer_alloc(0, &lpModule->pSpiDev->dev);
    if (!lpModule->pFbInfo) {
        return -ENOMEM;
    }

    lpModule->pFbInfo->var = gsFbVarScreenInfo;
    lpModule->pFbInfo->fix = gsFbFixScreenInfo;
    lpModule->pFbInfo->fbops = &gsFbOps;
    
    // Create a virtual memory
    #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
    if (!lpModule->pSpiDev->dev.dma_mask)
    {
        lpModule->pSpiDev->dev.dma_mask =
            &lpModule->pSpiDev->dev.coherent_dma_mask;
    }
    else
    {
        // Do nothing
    }
    lpModule->pSpiDev->dev.coherent_dma_mask = DMA_BIT_MASK(64);

    lpModule->pFbInfo->screen_base = dma_alloc_coherent(&lpModule->pSpiDev->dev,
        ILI9341_BUFFER_SIZE, &lpModule->sDmaHandle, GFP_KERNEL);
    if (0 == lpModule->pFbInfo->screen_base)
    {
        pr_err("[%s - %d] > dma_alloc_coherent failed\n", __func__, __LINE__);
        goto LABEL_RM_FB;
    }
    else
    {
        // Do nothing
    }

    lpModule->pFbInfo->fix.smem_start = lpModule->sDmaHandle;
    lpModule->pFbInfo->fix.smem_len = ILI9341_BUFFER_SIZE;

    lpModule->pFbInfo->fbdefio = &gsILI9341DefIo;
    fb_deferred_io_init(lpModule->pFbInfo);

    lpModule->pFbInfo->flags = FBINFO_HWACCEL_DISABLED | FBINFO_VIRTFB; // Defaut value
    #endif /* End of #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON) */

    if (register_framebuffer(lpModule->pFbInfo) < 0)
    {
        pr_info("[%s - %d] > Register framebuffer failed!\n", __func__, __LINE__);
        goto LABEL_RM_FB;
    }

    /* Store driver data in frame buffer instance */
    // fb_set_drvdata(lpModule->pFbInfo, lpModule);

    pr_info("[%s - %d] > End create framebuffer file!\n", __func__, __LINE__);
    return 0;

LABEL_RM_FB:
    framebuffer_release(lpModule->pFbInfo);
    return -EINVAL;
}

// This function will be called when we open the Frame Buffer
static int ILI9341_FbOpen(struct fb_info *lpInfo, int liUser)
{
    pr_info("[%s - %d] > System Call open() was called!\n", __func__, __LINE__);
    return 0;
}

// This function will be called when we close the Frame Buffer
static int ILI9341_FbRelease(struct fb_info *lpInfo, int liUser)
{
    pr_info("[%s - %d] > System Call close() was called!\n", __func__, __LINE__);
    return 0;
}

static ssize_t ILI9341_FbWrite(struct fb_info *info, const char __user *buf, size_t count, loff_t *ppos)
{
    pr_info("[%s - %d] > System Call was called!\n", __func__, __LINE__);

    // ILI9341_SPIModule_t *lpModule = fb_get_drvdata(lpInfo);
    char *lpDataPtr = kmalloc(count, GFP_KERNEL);

    /* Copy from user buffer to mapped area */
    memset(lpDataPtr, 0, count);
    if (copy_from_user(lpDataPtr, buf, count) != 0)
    {
        return -EFAULT;
    }
    pr_info("[%s - %d] > Received %d byte from user, %d!\n",
        __func__, __LINE__, (int)count, (int)(*ppos));

    // Send Data to LCD
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_RAMWR);
    for (int liIndex = 0; liIndex < count; liIndex++)
    {
        ILI9341_SpiSendByte(ILI9341_MODE_DATA, lpDataPtr[liIndex]);
    }

    kfree(lpDataPtr);
    return count;    
}

static int ILI9341_FbCheckVar(struct fb_var_screeninfo *var, struct fb_info *info)
{
    pr_info("[%s - %d] > System Call Check Var was called!\n", __func__, __LINE__);
    return 0;
}

static int ILI9341_FbSetPar(struct fb_info *info)
{
    pr_info("[%s - %d] > System Call Set Par was called!\n", __func__, __LINE__);
    return 0;    
}

static int ILI9341_FbSetColReg(unsigned regno, unsigned red, unsigned green, unsigned blue, unsigned transp, struct fb_info *info)
{
    pr_info("[%s - %d] > System Call Set Color Register was called!\n", __func__, __LINE__);
    return 0;
}

static int ILI9341_FbMmap(struct fb_info *info, struct vm_area_struct *vma)
{
    int liReturnValue;
    pr_info("[%s - %d] > System Call Memory Map was called!\n", __func__, __LINE__);
    
    #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
    unsigned long lulVmStart = vma->vm_start;
    unsigned long lulSize = vma->vm_end - vma->vm_start;
    unsigned long lulOffset = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long lilPfn;

    void *lpScreenBase = gpModuleILI9341->pFbInfo->screen_base;
    size_t lsSmemLen   = gpModuleILI9341->pFbInfo->fix.smem_len;
    
    if ((lulOffset > lsSmemLen) || (lulSize > (lsSmemLen - lulOffset)))
    {
        pr_err("[%s - %d] > mmap out of bounds: offset=%lu, size=%lu, smem_len=%lu\n",
            __func__, __LINE__, lulOffset, lulSize, lsSmemLen);
        return -EINVAL;
    }

    lilPfn = virt_to_phys(lpScreenBase + lulOffset) >> PAGE_SHIFT;

    // Set VMA page protection for non-cached memory (important for DMA)
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    // if ((vma->vm_end - vma->vm_start) > lulSize)
    // {
    //     pr_err("[%s - %d] > wrong size %lu %lu %lu\n", __func__, __LINE__,
    //         vma->vm_end, vma->vm_start, lulSize);
    //     return -EINVAL;
    // }

    // vma->vm_flags = (vma->vm_flags | VM_IO | VM_DONTEXPAND | VM_DONTDUMP) & ~VM_MAYEXEC;
    // vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

    liReturnValue = remap_pfn_range(vma, lulVmStart, lilPfn, lulSize, vma->vm_page_prot);
    if (0 != liReturnValue)
    {
        pr_err("[%s - %d] > remap_pfn_range failed: %d\n", __func__, __LINE__, liReturnValue);
        return -EAGAIN;
    }
    else
    {
        // Do nothing
    }
    // ILI9341_FbDmaFlush(gpModuleILI9341);
    pr_info("[%s - %d] > Success!\n", __func__, __LINE__);
    #endif /* End of #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON) */

    return 0;
}

#if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
static void ILI9341_DeferredIo(struct fb_info *info, struct list_head *pagelist)
{
    pr_info("[%s - %d] > deferred_io: triggering SPI+DMA update\n", __func__, __LINE__);
    // Call your display update function here
    // spi_fb_update(gpModuleILI9341);
}

static void ILI9341_FbDmaFlush(ILI9341_SPIModule_t *lpModule)
{
    struct dma_async_tx_descriptor *lpTxDesc;
    dma_addr_t lsDmaSrc = (dma_addr_t)lpModule->pFbInfo->screen_base;

    // Setup SPI/DMA transfer here using your SPI controller’s DMA engine
    ILI9341_SpiWriteDma((void *)lsDmaSrc, ILI9341_BUFFER_SIZE);
}
#endif /* End of #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON) */

static void ILI9341_FbFillRect(struct fb_info *info, const struct fb_fillrect *rect)
{
    pr_info("[%s - %d] > System Call was called!\n", __func__, __LINE__);
}

static void ILI9341_FbCopyArea(struct fb_info *info, const struct fb_copyarea *region)
{
    pr_info("[%s - %d] > System Call was called!\n", __func__, __LINE__);
}

#if (ILI9341_FB_IMAGEBLIT_ENABLE == ILI9341_ON)
static void ILI9341_FbImageBlit(struct fb_info *info, const struct fb_image *image)
{
    pr_info("[%s - %d] > System Call was called!\n", __func__, __LINE__);
    
    int liSizePicture = image->width * image->height * image->depth;
    pr_info("[%s - %d] > liSizePicture = %d\n", __func__, __LINE__, liSizePicture);

    // ILI9341_SPIModule_t *lpModule = fb_get_drvdata(lpInfo);
    char *lpDataPtr = kmalloc(liSizePicture, GFP_KERNEL);

    /* Copy from user buffer to mapped area */
    memset(lpDataPtr, 0, liSizePicture);
    if (copy_from_user(lpDataPtr, image->data, liSizePicture) != 0)
    {
        kfree(lpDataPtr);
        return;
    }

    // Send Data to LCD
    pr_info("[%s - %d] > Send Data to LCD!\n", __func__, __LINE__);
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_RAMWR);
    for (int liIndex = 0; liIndex < liSizePicture; liIndex++)
    {
        ILI9341_SpiSendByte(ILI9341_MODE_DATA, lpDataPtr[liIndex]);
    }

    kfree(lpDataPtr);
}
#endif /* End of #if (ILI9341_FB_IMAGEBLIT_ENABLE == ILI9341_ON) */

static int ILI9341_FbBlank(int blank_mode, struct fb_info *info)
{
    pr_info("[%s - %d] > System Call was called!\n", __func__, __LINE__);
    return 0;
}
#endif /* End of #if (ILI9341_FB_CREATION == ILI9341_ON) */

/*****************************************************************************************
 * The function to handle the device matching
 ****************************************************************************************/
static int ILI9341_SpiProbe(struct spi_device *lpSpi)
{
    int liReturnValue;
    ILI9341_SPIModule_t *lpModule;

    struct device_node *lpSpiDeviceNode = lpSpi->dev.of_node;

    pr_info("[%s - %d] > Probing ILI9341!\n", __func__, __LINE__);

    lpModule = kmalloc(sizeof(*lpModule), GFP_KERNEL);
    if (!lpModule)
    {
        pr_err("[%s - %d] > kmalloc failed!\n", __func__, __LINE__);
        return -ENOMEM;
    }
    else
    {
        // Do nothing
    }

    /* Get GPIO pins from device tree */
    lpModule->iRstPin = of_get_named_gpio(lpSpiDeviceNode, "reset-gpios", 0);
    if (0 > lpModule->iRstPin)
    {
        pr_err("[%s - %d] > Failed to get reset-gpios from DT: %d\n",
            __func__, __LINE__, lpModule->iRstPin);
        liReturnValue = lpModule->iRstPin;
        goto LABEL_ERR_FREE_MODULE;
    }
    else
    {
        // Do nothing
    }
    
    lpModule->iDcPin = of_get_named_gpio(lpSpiDeviceNode, "dc-gpios", 0);
    if (0 > lpModule->iDcPin)
    {
        pr_err("[%s - %d] > Failed to get dc-gpios from DT: %d\n", 
            __func__, __LINE__, lpModule->iDcPin);
        liReturnValue = lpModule->iDcPin;
        goto LABEL_ERR_FREE_MODULE;
    }
    else
    {
        // Do nothing
    }
    
    pr_info("[%s - %d] > GPIO pins: RST=%d, DC=%d\n", 
        __func__, __LINE__, lpModule->iRstPin, lpModule->iDcPin);

    lpSpi->mode = SPI_MODE_0;
    lpSpi->bits_per_word = 8;
    if (!lpSpi->max_speed_hz)
    {
        lpSpi->max_speed_hz = ILI9341_SPI_SPEED; /* SPI speed */
    }
    
    // Setup the SPI
    liReturnValue = spi_setup(lpSpi);
    if (0 > liReturnValue)
    {
        pr_err("[%s - %d] > Failed to setup SPI: %d\n", __func__, __LINE__, liReturnValue);
        goto LABEL_ERR_FREE_MODULE;
    }
    else
    {
        // Do nothing
    }

    /* Store SPI device in module structure */
    lpModule->pSpiDev = lpSpi;
    gpModuleILI9341 = lpModule;
    
    pr_info("[%s - %d] > Probe success!\n", __func__, __LINE__);
    
    /* Create device file */
    #if (ILI9341_CDEV_CREATION == ILI9341_ON)
    liReturnValue = ILI9341_CreateDeviceFile(lpModule);
    if (0 != liReturnValue)
    {
        pr_err("[%s - %d] > create device file failed!\n", __func__, __LINE__);
        goto LABEL_ERR_FREE_MODULE;
    }
    else
    {
        // Do nothing
    }
    #endif

    // Create FB file
    #if (ILI9341_FB_CREATION == ILI9341_ON)
    liReturnValue = ILI9341_CreateFbFile(lpModule);
    if (0 != liReturnValue)
    {
        pr_err("[%s - %d] > create Frame Buffer file failed!\n", __func__, __LINE__);
        goto LABEL_ERR_FREE_MODULE;
    }
    else
    {
        // Do nothing
    }
    #endif
    
    dev_info(&lpModule->pSpiDev->dev, "Framebuffer allocated:\n");
    dev_info(&lpModule->pSpiDev->dev, "  CPU addr: %p\n", lpModule->pFbInfo->screen_base);
    #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
    dev_info(&lpModule->pSpiDev->dev, "  DMA addr: %pad\n", &lpModule->sDmaHandle);
    #endif
    
    /* Store driver data in SPI device */
    spi_set_drvdata(lpSpi, lpModule);

    /* USER CODE START */
    ILI9341_DisplayInit(gpModuleILI9341);
    pr_info("[%s - %d] > LCD Init success!\n", __func__, __LINE__);
    /* USER CODE END */

    return 0;

LABEL_ERR_FREE_MODULE:
    kfree(lpModule);
    return liReturnValue;
}

static void ILI9341_SpiRemove(struct spi_device *lpSpi)
{
    ILI9341_SPIModule_t *lpModule = spi_get_drvdata(lpSpi);

    pr_info("[%s - %d] > Removing LCD SPI device\n", __func__, __LINE__);

    /* USER CODE START */
    /* USER CODE END */

    // Remove the device file and driver
    #if (ILI9341_CDEV_CREATION == ILI9341_ON)
    cdev_del(&lpModule->pCdev);
    device_destroy(lpModule->pClass, lpModule->sDevNum);
    class_destroy(lpModule->pClass);
    unregister_chrdev_region(lpModule->sDevNum, 1);
    #endif

    #if (ILI9341_FB_CREATION == ILI9341_ON)
    unregister_framebuffer(gpModuleILI9341->pFbInfo);
    #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
    dma_free_coherent(&lpModule->pSpiDev->dev, ILI9341_BUFFER_SIZE,
        &lpModule->pFbInfo->screen_base, lpModule->sDmaHandle);
    #endif
    #endif
    
    ILI9341_DisplayDeinit(lpModule);
    pr_info("[%s - %d] > driver remove!\n", __func__, __LINE__);
    
    kfree(lpModule);
    gpModuleILI9341 = NULL;
}

// Matching the device tree
MODULE_DEVICE_TABLE(of, ili9341_of_match_ids);
MODULE_DEVICE_TABLE(spi, ili9341_spi_match_ids);  // Correct for SPI legacy matching
module_spi_driver(gsILI9341SpiDriver);

/*****************************************************************************************
 * The macro to register module information
 ****************************************************************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERS);

/*****************************************************************************************
 * The function to control the LCD
 ****************************************************************************************/
#if (ILI9341_FB_CREATION == ILI9341_ON)
#if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
static void spi_dma_done(void *context)
{
    pr_info("[%s - %d] > DMA SPI transfer completed!\n", __func__, __LINE__);
}

static int ILI9341_SpiWriteDma(void *lpBuf, size_t liLen)
{
    pr_info("[%s - %d] > DMA SPI transfer start!\n", __func__, __LINE__);
    struct spi_transfer lsSpiTransfer = {
        .tx_buf = lpBuf,
        .len = liLen,
        .speed_hz = ILI9341_SPI_SPEED,
        // .tx_dma = gpModuleILI9341->sDmaHandle;
    };

    /* Set DC pin according to data mode */
    gpio_set_value(gpModuleILI9341->iDcPin, GPIO_HIGH);

    struct spi_message lsSpiMessage;
    lsSpiMessage.complete  = spi_dma_done;
    spi_message_init(&lsSpiMessage);
    spi_message_add_tail(&lsSpiTransfer, &lsSpiMessage);

    return spi_async(gpModuleILI9341->pSpiDev, &lsSpiMessage);  // or spi_async() if non-blocking
}
#endif /* End of #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON) */
#endif /* End of #if (ILI9341_FB_CREATION == ILI9341_ON) */

 static int ILI9341_SpiSendByte(bool lbIsData, unsigned char lcData)
{
    int liReturnValue;
    struct spi_transfer lsSpiTransfer;
    struct spi_message lsSpiMessage;

    /* Set DC pin according to data/command mode */
    gpio_set_value(gpModuleILI9341->iDcPin, lbIsData ? GPIO_HIGH : GPIO_LOW);
    
    /* Initialize SPI message */
    memset(&lsSpiTransfer, 0, sizeof(lsSpiTransfer));
    spi_message_init(&lsSpiMessage);
    
    /* Set up transfer */
    lsSpiTransfer.tx_buf = &lcData;
    lsSpiTransfer.len = 1;
    lsSpiTransfer.speed_hz = ILI9341_SPI_SPEED;  /* 4MHz */
    spi_message_add_tail(&lsSpiTransfer, &lsSpiMessage);
    
    /* Perform transfer */
    liReturnValue = spi_sync(gpModuleILI9341->pSpiDev, &lsSpiMessage);
    if (0 > liReturnValue)
    {
        pr_err("[%s - %d] > SPI transfer failed: %d\n",
            __func__, __LINE__, liReturnValue);
    }
    else
    {
        // Do nothing
    }
    
    /* Add small delay for stability */
    // udelay(1);

    return liReturnValue;
}

static int ILI9341_SpiSendArray(bool lbIsData, unsigned char * lpData, unsigned int liLen)
{
    int liReturnValue;
    struct spi_transfer lsSpiTransfer;
    struct spi_message lsSpiMessage;

    /* Set DC pin according to data/command mode */
    gpio_set_value(gpModuleILI9341->iDcPin, lbIsData ? GPIO_HIGH : GPIO_LOW);
    
    /* Initialize SPI message */
    memset(&lsSpiTransfer, 0, sizeof(lsSpiTransfer));
    spi_message_init(&lsSpiMessage);
    
    /* Set up transfer */
    lsSpiTransfer.tx_buf = lpData;
    lsSpiTransfer.len = liLen;
    lsSpiTransfer.speed_hz = ILI9341_SPI_SPEED;  /* 4MHz */
    spi_message_add_tail(&lsSpiTransfer, &lsSpiMessage);
    
    /* Perform transfer */
    liReturnValue = spi_sync(gpModuleILI9341->pSpiDev, &lsSpiMessage);
    if (0 > liReturnValue)
    {
        pr_err("[%s - %d] > SPI transfer failed: %d\n",
            __func__, __LINE__, liReturnValue);
    }
    else
    {
        // Do nothing
    }
    
    /* Add small delay for stability */
    udelay(1);

    return liReturnValue;
}

 static int ILI9341_DisplayInit(ILI9341_SPIModule_t *module)
{
    int liReturnValue;
    pr_info("[%s - %d] > ILI9341 display initialization\n", __func__, __LINE__);

    /* Request GPIO pins */
    liReturnValue = gpio_request(gpModuleILI9341->iRstPin, "RST");
    if (liReturnValue)
    {
        pr_err("[%s - %d] > Failed to request RST GPIO: %d\n", __func__, __LINE__, liReturnValue);
        return liReturnValue;
    }
    else
    {
        // Do nothing
    }
    
    liReturnValue = gpio_request(gpModuleILI9341->iDcPin, "DC");
    if (liReturnValue)
    {
        pr_err("[%s - %d] > Failed to request DC GPIO: %d\n", __func__, __LINE__, liReturnValue);
        gpio_free(gpModuleILI9341->iRstPin);
        return liReturnValue;
    }
    else
    {
        // Do nothing
    }
    
    /* Set GPIO directions */
    gpio_direction_output(gpModuleILI9341->iRstPin, GPIO_LOW);
    gpio_direction_output(gpModuleILI9341->iDcPin, GPIO_LOW);

    /* Reset LCD */
    gpio_set_value(gpModuleILI9341->iRstPin, GPIO_LOW);
    mdelay(10);  /* Longer reset pulse for reliability */
    gpio_set_value(gpModuleILI9341->iRstPin, GPIO_HIGH);
    mdelay(10);  /* Allow LCD to stabilize */

    /* INIT COMMAND SEQUENCE START */
    // Manufacturer command
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, 0xEF);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x03);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x80);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x02);

    // Power Control B
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, 0xCF);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0xC1);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x30);

    // Power on sequence control
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, 0xED);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x64);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x03);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x12);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x81);

    // Driver timing control A
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, 0xE8);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x85);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x78);

    // Power control A
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, 0xCB);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x39);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x2C);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x34);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x02);

    // Pump ratio control
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, 0xF7);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x20);

    // Driver timing control B
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, 0xEA);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);

    // Power control 1
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_PWCTR1);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x23);

    // Power control 2
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_PWCTR2);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x10);

    // VCOM control 1
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_VMCTR1);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x3E);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x28);

    // VCOM control 2
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_VMCTR2);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x86);

    // Memory Access Control
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_MADCTL);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0xE0);     // MY, MX, MV, RGB mode

    // Pixel Format Set
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_PIXFMT);  
    
    #if (ILI9341_BIT_PER_PIXEL == 18)
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x66);     // 18-bit color  
    #elif (ILI9341_BIT_PER_PIXEL == 16)
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x55);     // 16-bit color
    #endif

    // Frame Rate Control
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_FRMCTR1);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x18);

    // Display Function Control
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_DFUNCTR);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x08);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x82);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x27);

    // 3Gamma Function Disable
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, 0xF2);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);

    // Gamma Curve Selected
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_GAMMASET);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x01);

    // Positive Gamma Correction
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_GMCTRP1);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x0F);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x31);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x2B);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x0C);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x0E);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x08);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x4E);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0xF1);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x37);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x07);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x10);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x03);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x0E);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x09);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);

    // Negative Gamma Correction
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_GMCTRN1);  
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x0E);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x14);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x03);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x11);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x07);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x31);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0xC1);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x48);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x08);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x0F);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x0C);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x31);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x36);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x0F);
    
    // Set column address (now Y range after MV)
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_PASET);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);        // Start column = 0
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0xEF);        // End column = 239 (240 pixels)

    // Set page address (now X range after MV)
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_CASET);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);        // Start row = 0
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x01);
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x3F);        // End row = 319 (320 pixels)

    // Exit Sleep
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_SLPOUT);  
    msleep(120);

    // Display ON
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_DISPON);
    /* INIT COMMAND SEQUENCE END */

    /* TESTING LCD START */
    mdelay(100);  /* Allow LCD to stabilize */
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_RAMWR);
    for (int i = 0; i < (ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT); i++)
    {
        unsigned char lowbyte = (unsigned char)(i & 0x00FF);
        unsigned char highbyte = (unsigned char)((i >> 8) & 0x00FF);
        // unsigned char lowbyte = 0xF8;
        // unsigned char highbyte = 0x00;
        ILI9341_SpiSendByte(ILI9341_MODE_DATA, lowbyte);
        ILI9341_SpiSendByte(ILI9341_MODE_DATA, highbyte);
        // mdelay(1);  /* Allow LCD to stabilize */
    }
    /* TESTING LCD END */

    return 0;
}

static void ILI9341_DisplayDeinit(ILI9341_SPIModule_t *module)
{
    /* Reset LCD */
    gpio_set_value(module->iRstPin, GPIO_LOW);
    mdelay(10);  /* Longer reset pulse for reliability */
    gpio_set_value(module->iRstPin, GPIO_HIGH);
    mdelay(10);  /* Allow LCD to stabilize */

    gpio_free(module->iRstPin);
    gpio_free(module->iDcPin);
}
