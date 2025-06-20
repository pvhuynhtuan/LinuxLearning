#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>

// Convert 24-bit RGB to 16-bit RGB565
uint16_t rgb24_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Read BMP and write to framebuffer
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <image.bmp> <framebuffer device>\n", argv[0]);
        return 1;
    }

    const char *bmp_path = argv[1];
    const char *fb_path = argv[2];

    FILE *bmp = fopen(bmp_path, "rb");
    if (!bmp) {
        perror("fopen bmp");
        return 1;
    }

    // Read BMP header
    uint8_t header[54];
    fread(header, sizeof(header), 1, bmp);

    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int offset = *(int*)&header[10];
    int bpp = *(short*)&header[28];

    if (bpp != 24) {
        fprintf(stderr, "Only 24-bit BMP is supported.\n");
        fclose(bmp);
        return 1;
    }

    int row_padded = (width * 3 + 3) & (~3);
    uint8_t *row = malloc(row_padded);

    // Open framebuffer
    int fb = open(fb_path, O_RDWR);
    if (fb == -1) {
        perror("open framebuffer");
        free(row);
        fclose(bmp);
        return 1;
    }

    struct fb_var_screeninfo vinfo;
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);

    if (vinfo.bits_per_pixel != 16) {
        fprintf(stderr, "Only 16bpp (RGB565) framebuffer supported.\n");
        close(fb);
        free(row);
        fclose(bmp);
        return 1;
    }

    size_t screensize = vinfo.yres * vinfo.xres * 2;
    uint16_t *fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);

    if ((intptr_t)fbp == -1) {
        perror("mmap");
        close(fb);
        free(row);
        fclose(bmp);
        return 1;
    }

    // Move file pointer to start of pixel data
    fseek(bmp, offset, SEEK_SET);

    // Write pixels to framebuffer (BMP is bottom-up)
    for (int y = 0; y < height; y++) {
        fread(row, 1, row_padded, bmp);
        for (int x = 0; x < width; x++) {
            uint8_t b = row[x * 3 + 0];
            uint8_t g = row[x * 3 + 1];
            uint8_t r = row[x * 3 + 2];
            uint16_t color = rgb24_to_rgb565(r, g, b);

            int fb_y = height - y - 1; // BMP stores pixels bottom-up
            if (fb_y < vinfo.yres && x < vinfo.xres) {
                fbp[fb_y * vinfo.xres + x] = color;
            }
        }
    }

    munmap(fbp, screensize);
    close(fb);
    free(row);
    fclose(bmp);
    return 0;
}
