#include "gl.h"

void gl_init(unsigned int width, unsigned int height, unsigned int mode)
{
    fb_init(width, height, 4, mode);
}

void gl_swap_buffer(void)
{
    fb_swap_buffer();
}

unsigned int gl_get_width(void) 
{
    return fb_get_width();
}

unsigned int gl_get_height(void)
{
    return fb_get_height();
}

unsigned int gl_get_pitch(void)
{
    return fb_get_pitch();
}

color_t gl_color(unsigned char r, unsigned char g, unsigned char b)
{
    color_t color = color | (0xff << 24);
    color = color | (r << 16);
    color = color | (g << 8);
    color = color | (b);
    return color;
}

void gl_clear(color_t c)
{
    for (int i = 0; i < gl_get_width(); i++) {
        for (int j = 0; j < gl_get_height(); j++) {
            gl_draw_pixel(i, j, c);
        }
    }
}

void gl_draw_pixel(int x, int y, color_t c)
{
    int pitch = gl_get_pitch();
    unsigned (*im)[pitch / 4] = (unsigned (*)[pitch / 4])fb_get_draw_buffer();
    im[y][x] = c;
}

color_t gl_read_pixel(int x, int y)
{
    int pitch = gl_get_pitch();
    unsigned (*im)[pitch / 4] = (unsigned (*)[pitch / 4])fb_get_draw_buffer();
    return im[y][x];
}

void gl_draw_rect(int x, int y, int w, int h, color_t c)
{
    for (int i = 0; i < h; i++) {
        for( int j = 0; j < w; j++ ) {
            if (i + x < gl_get_width() && j + y < gl_get_height()) {
                gl_draw_pixel(i + x, j + y, c);
            }
        }
    }
}

void gl_draw_char(int x, int y, int ch, color_t c)
{
    int font_size = font_get_size();
    int font_width = font_get_width();
    unsigned char img[font_size];
    font_get_char(ch, img, sizeof(img));
    
    int originalx = x;
    for (int i = 0; i < font_size; i++) {
        if (img[i] == 0xff) {
            if (x < gl_get_width() && y < gl_get_height()) {
                gl_draw_pixel(x, y, c);
            }
        }
        if ((i+1) % font_width == 0) {
            y++;
            x = originalx;
        } else {
            x++;
        }
    }
}

void gl_draw_string(int x, int y, char* str, color_t c)
{
    int string_size = strlen(str);
    for (int i = 0; i < string_size; i++) {
        if (x < gl_get_width() && y < gl_get_height()) {
            gl_draw_char(x, y, str[i], c);
        }
        x+= font_get_width();
    }
    
}

unsigned int gl_get_char_height(void)
{
    return font_get_height();
}

unsigned int gl_get_char_width(void)
{
    return font_get_width();
}

