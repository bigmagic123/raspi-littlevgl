#ifndef DRV_DRV_DB_H__
#define DRV_DRV_DB_H__

#include <stdint.h>
#include <rtthread.h>

typedef uint16_t color_t;

#define   BLACK     0x0000        //   黑色    
#define   NAVY      0x000F        //   深蓝色  
#define   DGREEN    0x03E0        //   深绿色  
#define   DCYAN     0x03EF        //   深青色  
#define   MAROON    0x7800        //   深红色      
#define   PURPLE    0x780F        //   紫色  
#define   OLIVE     0x7BE0        //   橄榄绿      
#define   LGRAY     0xC618        //   灰白色
#define   DGRAY     0x7BEF        //   深灰色      
#define   BLUE      0x001F        //   蓝色    
#define   GREEN     0x07E0        //   绿色          
#define   CYAN      0x07FF        //   青色  
#define   RED       0xF800        //   红色       
#define   MAGENTA   0xF81F        //   品红    
#define   YELLOW    0xFFE0        //   黄色        
#define   WHITE     0xFFFF        //   白色  

/**
 * Creates a color_t with the given rgb values.
 */
#define RGB(red, green, blue)\
    ((color_t)((red << 11) | (green << 5) | (blue << 0)))

/**
 * Initialises a 32 bit framebuffer with the given width and height.
 */
rt_err_t rt_fb_init(uint32_t width, uint32_t height);

/**
 * Clears the framebuffer with the given color.
 */
void fb_clear(color_t color);

/**
 * Draws a line between two points.
 */
void fb_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, color_t color);

/**
 * Draws an outlined rectangle to the screen.
 */
void fb_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, color_t color);

/**
 * Draws a filled in rectangle to the screen.
 */
void fb_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, color_t color);

/**
 * Draws an image specified by the GIMP header image format to the screen.
 */
void fb_draw_image(uint32_t x, uint32_t y, uint32_t width, uint32_t height, char* pixels);

#endif