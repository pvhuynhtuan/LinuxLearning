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
* Install by depmod:
*   1. Copy the ko file to system:
*       sudo mkdir -p /lib/modules/$(uname -r)/extra
*       sudo cp ili9341.ko /lib/modules/$(uname -r)/extra/
*   2. sudo depmod -a
*   3. If the compatible = "ili9341", the default driver of ili9341 shal be install automatically
*       ==> Remove ko file: kernel/drivers/staging/fbtft/fb_ili9341.ko.xz
        ==> execute: "sudo depmod -a" again
* Kill all process which using the frame buffer device: sudo fuser -k /dev/fb1
* Sample command to test screen: sudo fbi -T 1 -d /dev/fb1 -a -noverbose ~/FinalProject/00_Common/vietnam.jpg
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
#include <linux/vmalloc.h> /* For Non-DMA */

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

#define ILI9341_FB_DMA_ENABLE   ILI9341_OFF

#define ILI9341_FB_COLOR_SWAP   ILI9341_ON
#define ILI9341_FB_LCD_TEST     ILI9341_OFF


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
#define ILI9341_SPI_SPEED       16000000

/* LCD modes */
#define ILI9341_MODE_CMD        0
#define ILI9341_MODE_DATA       1

#define ILI9341_TFTWIDTH        320 ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT       240 ///< ILI9341 max TFT height
#define ILI9341_BIT_PER_PIXEL   16

#define ILI9341_FB_SIZE_RAW     (ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT * ((ILI9341_BIT_PER_PIXEL +7) / 8)) //Round-up the byte

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
    #else
    unsigned int iPagesCount;
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
static void ILI9341_FbImageBlit(struct fb_info *info, const struct fb_image *image);
static int ILI9341_FbBlank(int blank_mode, struct fb_info *info);
static void ILI9341_DeferredIo(struct fb_info *info, struct list_head *pagelist);
#if (ILI9341_FB_DMA_ENABLE == ILI9341_OFF)
static int ILI9341_VideoAlloc(ILI9341_SPIModule_t *lpModule);
#endif
#endif /* End of #if (ILI9341_FB_CREATION == ILI9341_ON) */

static int ILI9341_SpiProbe(struct spi_device *lpSpi);
static void ILI9341_SpiRemove(struct spi_device *lpSpi);

// The function to control the LCD
#if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
static void spi_dma_done(void *context);
static void ILI9341_SpiDmaFlush(ILI9341_SPIModule_t *lpModule);
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
    { .compatible = "custom,ili9341", 0 },
    { }  /* sentinel */
};

static const struct spi_device_id ili9341_spi_match_ids[] = {
    { "custom,ili9341", 0 },
    { /* sentinel */ }
};

/* SPI driver structure */
static struct spi_driver gsILI9341SpiDriver = {
    .driver = {
        .name = "custom,ili9341",
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
    #if (ILI9341_BIT_PER_PIXEL == 16)
    .red.offset    = 11,
    .red.length    = 5,

    .green.offset  = 5,
    .green.length  = 6,

    .blue.offset   = 0,
    .blue.length   = 5,

    // .red.offset    = 0,
    // .red.length    = 5,

    // .green.offset  = 5,
    // .green.length  = 6,

    // .blue.offset   = 11,
    // .blue.length   = 5,

    .transp.offset = 0,
    .transp.length = 0,
    #endif
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
    #if (ILI9341_BIT_PER_PIXEL == 16)
    .line_length = (ILI9341_TFTWIDTH * ILI9341_BIT_PER_PIXEL) / 8, /* length of a line in bytes */
    #else
    .line_length = (ILI9341_TFTWIDTH * 3), /* length of a line in bytes */
    #endif
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
    .fb_imageblit = ILI9341_FbImageBlit, /* Draws a image to the display */
    .fb_blank = ILI9341_FbBlank, /* blank display */
};

static struct fb_deferred_io gsILI9341DefIo =
{
    .delay = HZ / 50,               // 50 ms
    .deferred_io = ILI9341_DeferredIo,
};
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
    #else
    /* Non-DMA virtual memory creation */
    if(ILI9341_VideoAlloc(lpModule) < 0)
    {
        pr_err("[%s - %d] > Create Virtual Memory - Non-DMA failed\n", __func__, __LINE__);
        goto LABEL_RM_FB;
    }
    else
    {
        // Do nothing
    }
    // Store the allocated memory start address
    lpModule->pFbInfo->screen_base =
        (char __iomem *)lpModule->pFbInfo->fix.smem_start;
    // pr_info("[%s - %d] > Create virtual memory success: screen_base=%d, smem_start=%d!\n",
    //     __func__, __LINE__,
    //     (unsigned int)lpModule->pFbInfo->screen_base,
    //     (unsigned int)lpModule->pFbInfo->fix.smem_start);
    #endif /* End of #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON) */

    lpModule->pFbInfo->fbdefio = &gsILI9341DefIo;
    if(fb_deferred_io_init(lpModule->pFbInfo) < 0)
    {
        pr_info("[%s - %d] > fb_deferred_io_init failed!\n", __func__, __LINE__);
        goto LABEL_RM_FB;
    }
    lpModule->pFbInfo->flags = FBINFO_HWACCEL_DISABLED | FBINFO_VIRTFB; // Defaut value
    

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
    // Force the resolution (optional — allow dynamic settings if desired)
    var->xres         = 320;
    var->yres         = 240;
    var->xres_virtual = 320;
    var->yres_virtual = 240;
    var->bits_per_pixel = 16;

    // RGB565 format
    var->red.offset   = 11;
    var->red.length   = 5;
    var->green.offset = 5;
    var->green.length = 6;
    var->blue.offset  = 0;
    var->blue.length  = 5;

    var->transp.offset = 0;
    var->transp.length = 0;

    // Set non-interlaced mode, progressive scan
    var->vmode = FB_VMODE_NONINTERLACED;
        
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
    // For true-color modes like RGB565, you usually don't need to store a palette.
    // But fbcon and Qt still expect this function to exist and succeed.

    if (regno >= 256) // Limit to standard 256 registers
    {
        return -EINVAL;
    }

    // If pseudo_palette is used (common in true-color), update it
    if (info->fix.visual == FB_VISUAL_TRUECOLOR && info->pseudo_palette) {
        uint16_t r = red >> (16 - info->var.red.length);
        uint16_t g = green >> (16 - info->var.green.length);
        uint16_t b = blue >> (16 - info->var.blue.length);

        uint32_t value = (r << info->var.red.offset) |
            (g << info->var.green.offset) |
            (b << info->var.blue.offset);

        ((uint32_t *)(info->pseudo_palette))[regno] = value;
    }
    return 0;
}

static int ILI9341_FbMmap(struct fb_info *info, struct vm_area_struct *vma)
{
    pr_info("[%s - %d] > System Call Memory Map was called!\n", __func__, __LINE__);
    
    #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
    int liReturnValue;
    unsigned long lulVmStart = vma->vm_start;
    void *lpScreenBase = gpModuleILI9341->pFbInfo->screen_base;
    #endif

    unsigned long lulSize = vma->vm_end - vma->vm_start;
    unsigned long lulOffset = vma->vm_pgoff << PAGE_SHIFT;
    size_t lsSmemLen   = gpModuleILI9341->pFbInfo->fix.smem_len;
    
    // Check the boundary address
    if ((lulOffset > lsSmemLen) || (lulSize > (lsSmemLen - lulOffset)))
    {
        pr_err("[%s - %d] > mmap out of bounds: offset=%lu, size=%lu, smem_len=%lu\n",
            __func__, __LINE__, lulOffset, lulSize, lsSmemLen);
        return -EINVAL;
    }
    else
    {
        pr_info("[%s - %d] > mmap: offset=%lu, size=%lu, smem_len=%lu\n",
            __func__, __LINE__, lulOffset, lulSize, lsSmemLen);
    }

    return fb_deferred_io_mmap(gpModuleILI9341->pFbInfo, vma);
}

static void ILI9341_FbFillRect(struct fb_info *info, const struct fb_fillrect *rect)
{
    pr_info("[%s - %d] > System Call was called!\n", __func__, __LINE__);
    
    uint8_t *lpDst;
    int liBpp = ((info->var.bits_per_pixel + 7) / 8); // Round-up the byte data
    uint32_t lulColor = rect->color;
    int liLineLen = info->fix.line_length;
    uint8_t lpColorBuf[4];  // Max size for 32bpp color
    
    pr_info("[%s - %d] > width = %d, height = %d\n",
        __func__, __LINE__, rect->width, rect->height);
    pr_info("[%s - %d] > image->dx = %d, image->dy = %d\n",
        __func__, __LINE__, rect->dx, rect->dy);
        
    // Prepare color buffer for memcpy
    memcpy(lpColorBuf, (void *)&lulColor, liBpp);

    for (int y = 0; y < rect->height; y++)
    {
        lpDst = info->screen_base + (rect->dy + y) * liLineLen + rect->dx * liBpp;

        for (int x = 0; x < rect->width; x++)
        {
            memcpy(lpDst + x * liBpp, lpColorBuf, liBpp);
        }
    }
        
    struct fb_deferred_io *lpFbDefIo = info->fbdefio;
	if (lpFbDefIo)
    {
		/* Schedule the deferred IO to kick in after a delay.*/
		schedule_delayed_work(&info->deferred_work, lpFbDefIo->delay);
	}
}

static void ILI9341_FbCopyArea(struct fb_info *info, const struct fb_copyarea *region)
{
    pr_info("[%s - %d] > System Call was called!\n", __func__, __LINE__);
    int liBpp = ((info->var.bits_per_pixel + 7) / 8); // Round-up the byte data
    int liLineLen = info->fix.line_length;
    uint8_t *lpSrc, *lpDst;

    int liBytesPerLine = region->width * liBpp;

    pr_info("[%s - %d] > width = %d, height = %d\n",
        __func__, __LINE__, region->width, region->height);
    pr_info("[%s - %d] > region->sx = %d, region->sy = %d, region->dx = %d, region->dy = %d\n",
        __func__, __LINE__, region->sx, region->sy, region->dx, region->dy);

    if (region->sy < region->dy)
    {
        // Copy bottom-up to avoid overwrite
        for (int y = region->height - 1; y >= 0; y--)
        {
            lpSrc = info->screen_base +
                  (region->sy + y) * liLineLen +
                  region->sx * liBpp;

            lpDst = info->screen_base +
                  (region->dy + y) * liLineLen +
                  region->dx * liBpp;

            memmove(lpDst, lpSrc, liBytesPerLine);
        }
    }
    else
    {
        // Copy top-down
        for (int y = 0; y < region->height; y++)
        {
            lpSrc = info->screen_base +
                  (region->sy + y) * liLineLen +
                  region->sx * liBpp;

            lpDst = info->screen_base +
                  (region->dy + y) * liLineLen +
                  region->dx * liBpp;

            memmove(lpDst, lpSrc, liBytesPerLine);
        }
    }
        
    struct fb_deferred_io *lpFbDefIo = info->fbdefio;
	if (lpFbDefIo)
    {
		/* Schedule the deferred IO to kick in after a delay.*/
		schedule_delayed_work(&info->deferred_work, lpFbDefIo->delay);
	}
}

static void ILI9341_FbImageBlit(struct fb_info *info, const struct fb_image *image)
{
    int liBit;
    uint8_t *lpDst;
    const uint8_t *lpSrc;
    int liLineLen = info->fix.line_length;
    uint32_t lulFgColor = image->fg_color;
    uint32_t lulBgColor = image->bg_color;
    // uint32_t lulFgColor = 0xFFFF;
    // uint32_t lulBgColor = 0x0000;
    uint32_t lulBpp = ((info->var.bits_per_pixel + 7) / 8);
    
    // pr_info("[%s - %d] > width = %d, height = %d, depth = %d\n",
    //     __func__, __LINE__, image->width, image->height, image->depth);
    // pr_info("[%s - %d] > lulFgColor = %d, lulBgColor = %d, lulBpp = %d, depth=%d\n",
    //     __func__, __LINE__, lulFgColor, lulBgColor, lulBpp, image->depth);
    // pr_info("[%s - %d] > image->dy = %d, image->dx = %d\n",
    //     __func__, __LINE__, image->dy, image->dx);
        
    if (image->depth == 1)
    {
        // Monochrome bitmap: each bit is a pixel
        // ==> Config the foreground/backgound color
        lulFgColor = 0xFFFF;
        lulBgColor = 0x0000;
        for (int y = 0; y < image->height; y++)
        {
            lpSrc = image->data + y * image->width / 8;
            lpDst = info->screen_base +
                (image->dy + y) * liLineLen +
                image->dx * lulBpp;

            for (int x = 0; x < image->width; x++)
            {
                liBit = lpSrc[x / 8] & (0x80 >> (x % 8));  // MSB-first bit unpacking

                uint32_t lulColor = liBit ? lulFgColor : lulBgColor;

                switch (lulBpp)
                {
                case 1:
                    *(uint8_t *)(lpDst + x) = lulColor;
                    break;
                case 2:
                    *(uint16_t *)(lpDst + x * 2) = lulColor;
                    break;
                case 4:
                    *(uint32_t *)(lpDst + x * 4) = lulColor;
                    break;
                default:
                    pr_warn("Unsupported bpp: %d\n", lulBpp * 8);
                    return;
                }
            }
        }
    }
    else
    {
        // For color bitmaps, just copy pixel data
        int liBytesPerPixel = image->depth / 8;

        for (int y = 0; y < image->height; y++)
        {
            lpDst = info->screen_base +
                  (image->dy + y) * liLineLen +
                  image->dx * liBytesPerPixel;

            lpSrc = image->data + y * image->width * liBytesPerPixel;

            memcpy(lpDst, lpSrc, image->width * liBytesPerPixel);
        }
    }
        
    struct fb_deferred_io *lpFbDefIo = info->fbdefio;
	if (lpFbDefIo)
    {
		/* Schedule the deferred IO to kick in after a delay.*/
		schedule_delayed_work(&info->deferred_work, lpFbDefIo->delay);
	}
    
}

static int ILI9341_FbBlank(int blank_mode, struct fb_info *info)
{
    pr_info("[%s - %d] > System Call was called!\n", __func__, __LINE__);
    return 0;
}

static void ILI9341_DeferredIo(struct fb_info *info, struct list_head *pagelist)
{
    // pr_info("[%s - %d] > deferred_io: triggering SPI+DMA update\n", __func__, __LINE__);
    // Call your display update function here
    uint8_t *lpMemAddr = info->screen_base;

    // Calculate the raw lenghth, not the buffer length
    size_t lsLen = gpModuleILI9341->pFbInfo->var.yres * 
        gpModuleILI9341->pFbInfo->fix.line_length;

    #if (ILI9341_FB_COLOR_SWAP == ILI9341_ON)
    uint8_t *lpSwapBuf = kmalloc(lsLen, GFP_KERNEL);
    for (int liIndex = 0; liIndex < lsLen; liIndex += 2)
    {
        lpSwapBuf[liIndex] = lpMemAddr[liIndex + 1];
        lpSwapBuf[liIndex + 1] = lpMemAddr[liIndex];
    }
    #endif

    // Send the write ram data command
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_RAMWR);

    // Send the data
    #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
    ILI9341_SpiDmaFlush(gpModuleILI9341);
    #endif

    #if (ILI9341_FB_COLOR_SWAP == ILI9341_ON)
    ILI9341_SpiSendArray(ILI9341_MODE_DATA, lpSwapBuf, lsLen);
    kfree(lpSwapBuf);
    #else
    ILI9341_SpiSendArray(ILI9341_MODE_DATA, lpMemAddr, lsLen);
    #endif
    // pr_info("[%s - %d] > deferred_io: completed\n", __func__, __LINE__);
}

#if (ILI9341_FB_DMA_ENABLE == ILI9341_OFF)
static int ILI9341_VideoAlloc(ILI9341_SPIModule_t *lpModule)
{
    unsigned int liFrameSize;

	pr_info("[%s - %d] > Initializing virtual memory...\n", __func__, __LINE__);

	liFrameSize = lpModule->pFbInfo->fix.line_length * lpModule->pFbInfo->var.yres;
	pr_info("[%s - %d] > liFrameSize=%u\n", __func__, __LINE__, liFrameSize);

	lpModule->iPagesCount = liFrameSize / PAGE_SIZE;
    // Rough the count
	if ((lpModule->iPagesCount * PAGE_SIZE) < liFrameSize)
    {
		lpModule->iPagesCount++;
	}
    pr_info("[%s - %d] > iPagesCount=%u\n", __func__, __LINE__, lpModule->iPagesCount);

	// lpModule->pFbInfo->fix.smem_len = ili->pages_count * PAGE_SIZE;
	lpModule->pFbInfo->fix.smem_start =
	    (unsigned long)vmalloc(lpModule->pFbInfo->fix.smem_len);
	if (!lpModule->pFbInfo->fix.smem_start)
    {
        pr_err("[%s - %d] > unable to vmalloc\n", __func__, __LINE__);
		return -ENOMEM;
	}
	memset((void *)lpModule->pFbInfo->fix.smem_start, 0,
        lpModule->pFbInfo->fix.smem_len);

	return 0;
}
#endif
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
    
    /* Store driver data in SPI device */
    spi_set_drvdata(lpSpi, lpModule);

    /* USER CODE START */
    ILI9341_DisplayInit(gpModuleILI9341);
    pr_info("[%s - %d] > LCD Init success!\n", __func__, __LINE__);
    
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
    #else
    vfree(lpModule->pFbInfo->screen_base);
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
#if (ILI9341_FB_DMA_ENABLE == ILI9341_ON)
static void spi_dma_done(void *context)
{
    pr_info("[%s - %d] > DMA SPI transfer completed!\n", __func__, __LINE__);
}

static void ILI9341_SpiDmaFlush(ILI9341_SPIModule_t *lpModule)
{
    pr_info("[%s - %d] > Dma Flush start!\n", __func__, __LINE__);
    struct spi_transfer lsSpiTransfer = {0};
    struct spi_message lsSpiMessage = {0};
    
    lsSpiTransfer.tx_buf = lpModule->pFbInfo->screen_base;
    lsSpiTransfer.tx_dma = lpModule->pFbInfo->fix.smem_start;
    lsSpiTransfer.len    = ILI9341_BUFFER_SIZE;
    lsSpiTransfer.speed_hz  = ILI9341_SPI_SPEED;

    /* Set DC pin according to data mode */
    gpio_set_value(gpModuleILI9341->iDcPin, GPIO_HIGH);

    spi_message_init(&lsSpiMessage);
    lsSpiMessage.complete  = spi_dma_done;
    lsSpiMessage.context  = gpModuleILI9341;
    spi_message_add_tail(&lsSpiTransfer, &lsSpiMessage);

    int liReturnValue = spi_async(lpModule->pSpiDev, &lsSpiMessage);
    if (liReturnValue)
    {
        pr_info("[%s - %d] > spi_async failed: %d\n", __func__, __LINE__, liReturnValue);
    }
        
}
#endif /* End of #if (ILI9341_FB_DMA_ENABLE == ILI9341_ON) */

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
    lsSpiTransfer.speed_hz = ILI9341_SPI_SPEED;
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

    // for (size_t i = 0; i < liLen; i += 2)
    // {
    //     uint8_t high = lpData[i];
    //     lpData[i] = lpData[i+1];
    //     lpData[i+1] = high;
    // }

    /* Set DC pin according to data/command mode */
    gpio_set_value(gpModuleILI9341->iDcPin, lbIsData ? GPIO_HIGH : GPIO_LOW);
    
    /* Initialize SPI message */
    memset(&lsSpiTransfer, 0, sizeof(lsSpiTransfer));
    spi_message_init(&lsSpiMessage);
    
    /* Set up transfer */
    lsSpiTransfer.tx_buf = lpData;
    lsSpiTransfer.len = liLen;
    lsSpiTransfer.speed_hz = ILI9341_SPI_SPEED;
    lsSpiTransfer.bits_per_word = 8;
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
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0xE8);     // MY, MX, MV, RGB mode

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
    ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x02);

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

    // Testint the interface control
    // ILI9341_SpiSendByte(ILI9341_MODE_CMD, 0xF6);
    // ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0xE0);
    // ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x00);
    // ILI9341_SpiSendByte(ILI9341_MODE_DATA, 0x0F);

    // Exit Sleep
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_SLPOUT);  
    msleep(120);

    // Display ON
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_DISPON);
    /* INIT COMMAND SEQUENCE END */

    /* TESTING LCD START */
    #if (ILI9341_FB_LCD_TEST == ILI9341_ON)
    mdelay(100);  /* Allow LCD to stabilize */

    unsigned char byteData[2];
    ILI9341_SpiSendByte(ILI9341_MODE_CMD, ILI9341_RAMWR);

    uint16_t liTempData = 0x8000;
    *((uint16_t *)byteData) = liTempData;
    pr_info("[%s - %d] > liTempData=0x%04X; byteData[0]=0x%02X; byteData[1]=0x%02X !\n",
        __func__, __LINE__, liTempData,byteData[0], byteData[1]);
    for (int i = 0; i < (ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT); i++)
    {
        // if (i < ((ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT) / 3))
        // {
        //     byteData[1] = (unsigned char)(ILI9341_RED & 0x00FF);
        //     byteData[0] = (unsigned char)((ILI9341_RED >> 8) & 0x00FF);
        // }
        // else if (i < (2 * ((ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT) / 3)))
        // {
        //     byteData[1] = (unsigned char)(ILI9341_GREEN & 0x00FF);
        //     byteData[0] = (unsigned char)((ILI9341_GREEN >> 8) & 0x00FF);
        // }
        // else
        // {
        //     byteData[1] = (unsigned char)(ILI9341_BLUE & 0x00FF);
        //     byteData[0] = (unsigned char)((ILI9341_BLUE >> 8) & 0x00FF);
        // }
        
        if ((i % 4800  == 0) && (i != 0))
        {
            liTempData >>= 1;
            *((uint16_t *)byteData) = liTempData;
            pr_info("[%s - %d] > liTempData=0x%04X; byteData[0]=0x%02X; byteData[1]=0x%02X !\n",
                __func__, __LINE__, liTempData,byteData[0], byteData[1]);
        }

        // byteData[0] = 0x00; // High byte
        // byteData[1] = 0x1F; // Low bye
        // ILI9341_SpiSendArray(ILI9341_MODE_DATA, byteData, 2);
        ILI9341_SpiSendByte(ILI9341_MODE_DATA, byteData[0]);
        ILI9341_SpiSendByte(ILI9341_MODE_DATA, byteData[1]);
    }
    // mdelay(500);  /* Allow LCD pause to see the test */
    #endif /* End of #if (ILI9341_FB_LCD_TEST == ILI9341_ON) */
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
