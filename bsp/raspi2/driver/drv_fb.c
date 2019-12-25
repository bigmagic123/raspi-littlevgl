#include "drv_fb.h"

#include "mailbox.h"

static uint32_t __attribute__((aligned(16))) mailbox_buffer[35];

#define MAILBOX_REQUEST 0x00000000
#define MAILBOX_SUCCESS 0x80000000

#define CHANNEL_PROP    8

#define TAG_ALLOCATE_BUFFER         0x00040001
#define TAG_SET_PHYS_WIDTH_HEIGHT   0x00048003
#define TAG_SET_VIRT_WIDTH_HEIGHT   0x00048004
#define TAG_SET_DEPTH               0x00048005
#define TAG_SET_PIXEL_ORDER         0x00048006
#define TAG_GET_PITCH               0x00040008
#define TAG_SET_VIRT_OFFSET         0x00048009
#define TAG_END                     0x00000000

#define COLOR_32_TO_16(src)\
    ((color_t)((((((src) >> 16) & 0xFF) >> 3) << 11) | (((((src) >> 8) & 0x01FF) >> 2) << 5) | (((src) & 0xFF) >> 3)))

static struct
{
    color_t* ptr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
} framebuffer = { 0 };

//添加fb init
rt_err_t rt_fb_init(uint32_t width, uint32_t height)
{
    mailbox_buffer[0] = sizeof(mailbox_buffer);
    mailbox_buffer[1] = MAILBOX_REQUEST;

    mailbox_buffer[2] = TAG_ALLOCATE_BUFFER;//get framebuffer, gets alignment on request
    mailbox_buffer[3] = 8;
    mailbox_buffer[4] = 0;
    mailbox_buffer[5] = 4096;   //FrameBufferInfo.pointer
    mailbox_buffer[6] = 0;      //FrameBufferInfo.size

    mailbox_buffer[7] = TAG_SET_PHYS_WIDTH_HEIGHT;
    mailbox_buffer[8] = 8;
    mailbox_buffer[9] = 0;
    mailbox_buffer[10] = width;
    mailbox_buffer[11] = height;

    mailbox_buffer[12] = TAG_SET_VIRT_WIDTH_HEIGHT;
    mailbox_buffer[13] = 8;
    mailbox_buffer[14] = 0;
    mailbox_buffer[15] = width;
    mailbox_buffer[16] = height;

    mailbox_buffer[17] = TAG_SET_DEPTH;
    mailbox_buffer[18] = 4;
    mailbox_buffer[19] = 0;
    mailbox_buffer[20] = 16;    //FrameBufferInfo.depth RGB 565

    mailbox_buffer[21] = TAG_SET_PIXEL_ORDER;
    mailbox_buffer[22] = 4;
    mailbox_buffer[23] = 0;
    mailbox_buffer[24] = 1;     //RGB, not BGR preferably

    mailbox_buffer[25] = TAG_GET_PITCH;
    mailbox_buffer[26] = 4;
    mailbox_buffer[27] = 0;
    mailbox_buffer[28] = 0;

    mailbox_buffer[29] = TAG_SET_VIRT_OFFSET;
    mailbox_buffer[30] = 8;
    mailbox_buffer[31] = 8;
    mailbox_buffer[32] = 0;
    mailbox_buffer[33] = 0;

    mailbox_buffer[34] = TAG_END;

    uint32_t mailbox_buffer_addr = (uint32_t)(uintptr_t)&mailbox_buffer;

    asm volatile ("dmb sy");
    mailbox_write(8, mailbox_buffer_addr);
    uint32_t result = mailbox_read(CHANNEL_PROP);
    asm volatile ("dmb sy");

    if (result != mailbox_buffer_addr || mailbox_buffer[1] != MAILBOX_SUCCESS)
    {
        return RT_ERROR;
    }

    framebuffer.ptr = (color_t*)(uintptr_t)(mailbox_buffer[5] & 0x3FFFFFFF);

    if (framebuffer.ptr == NULL)
    {
        return RT_ERROR;
    }

    framebuffer.width  = mailbox_buffer[10];
    framebuffer.height = mailbox_buffer[11];
    framebuffer.pitch  = mailbox_buffer[28] / sizeof(color_t);

    return RT_EOK;
}

void fb_clear(color_t color)
{
    color_t* ptr = framebuffer.ptr;

    if (ptr == NULL)
    {
        return;
    }

    for (uint32_t y = 0; y < framebuffer.height; y++)
    {
        for (uint32_t x = 0; x < framebuffer.width; x++)
        {
            *(ptr++) = color;
        }

        ptr += framebuffer.pitch - framebuffer.width;
    }
}

#define FB_PUT(x, y, color)\
    *(framebuffer.ptr + (framebuffer.pitch * y) + x) = color

void fb_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, color_t color)
{
    float xdiff = (int)(x2 - x1);
    float ydiff = (int)(y2 - y1);

    if (xdiff == 0 && ydiff == 0)
    {
        FB_PUT(x1, y1, color);
        return;
    }

    if ((xdiff) > (ydiff))
    {
        float xmin, xmax;

        if (x1 < x2)
        {
            xmin = x1;
            xmax = x2;
        }
        else
        {
            xmin = x2;
            xmax = x1;
        }

        float slope = ydiff / xdiff;

        for (int x = xmin; x <= xmax; x++)
        {
            int y = (int)y1 + ((x - (int)x1) * slope);
            FB_PUT(x, y, color);
        }
    }
    else
    {
        float ymin, ymax;

        if (y1 < y2)
        {
            ymin = y1;
            ymax = y2;
        }
        else
        {
            ymin = y2;
            ymax = y1;
        }

        float slope = xdiff / ydiff;

        for (int y = ymin; y <= ymax; y += 1.0f)
        {
            int x = (int)x1 + ((y - (int)y1) * slope);
            FB_PUT(x, y, color);
        }
    }
}

void fb_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, color_t color)
{
    color_t* ptr = framebuffer.ptr;

    if (ptr == NULL || x >= framebuffer.width || y >= framebuffer.height) return;

    if (x + width > framebuffer.width)
    {
        width = framebuffer.width - x;
    }

    if (y + height > framebuffer.height)
    {
        height = framebuffer.height - y;
    }

    fb_draw_line(x, y, x + width, y, color);
    fb_draw_line(x, y, x, y + height, color);
    fb_draw_line(x + width, y, x + width, y + height, color);
    fb_draw_line(x, y + height, x + width, y + height, color);
}

void fb_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, color_t color)
{
    color_t* ptr = framebuffer.ptr;

    if (ptr == NULL || x >= framebuffer.width || y >= framebuffer.height) return;

    if (x + width > framebuffer.width)
    {
        width = framebuffer.width - x;
    }

    if (y + height > framebuffer.height)
    {
        height = framebuffer.height - y;
    }

    ptr += (framebuffer.pitch * y) + x;

    for (uint32_t cy = y; cy < y + height; cy++)
    {
        uint32_t cx;

        for (cx = x; cx < x + width; cx++)
        {
            *(ptr++) = color;
        }

        ptr += (framebuffer.pitch - cx) + x;
    }
}

#define HEADER_PIXEL(data,pixel) {\
pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
data += 4; \
}

void fb_draw_image(uint32_t x, uint32_t y, uint32_t width, uint32_t height, char* data)
{
    color_t* ptr = framebuffer.ptr;

    if (ptr == NULL || x >= framebuffer.width || y >= framebuffer.height) return;

    if (x + width > framebuffer.width)
    {
        width = framebuffer.width - x;
    }

    if (y + height > framebuffer.height)
    {
        height = framebuffer.height - y;
    }

    ptr += (framebuffer.pitch * y) + x;

    char pixel[4] = { 255, 0, 0, 0 };

    for (uint32_t cy = 0; cy < height; cy++)
    {
        uint32_t cx;

        for (cx = 0; cx < width; cx++)
        {
            HEADER_PIXEL(data, (pixel + 1))
            *(ptr++) = COLOR_32_TO_16(*(uint32_t*)pixel);
        }

        ptr += framebuffer.pitch - cx;
    }
}