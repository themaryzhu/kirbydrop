#include "assert.h"
#include "timer.h"
#include "fb.h"
#include "gl.h"
#include "console.h"
#include "printf.h"

#define _WIDTH 600
#define _HEIGHT 500

#define _NROWS 20
#define _NCOLS 30

static void test_gl(void) {
    gl_init(_WIDTH, _HEIGHT, GL_DOUBLEBUFFER);
    gl_clear(gl_color(0xFF, 0, 0xFF)); // Background should be purple.
    gl_draw_pixel(_WIDTH/3, _HEIGHT/3, GL_AMBER);
    // assert(gl_read_pixel(_WIDTH/3, _HEIGHT/3) == GL_AMBER);
    // gl_draw_rect(_WIDTH/2 - 20, _HEIGHT/2 - 20, 4, 50, GL_BLUE);
    // gl_draw_char(60, 10, 'A', GL_BLUE);
    // gl_draw_string(300, 300, "Hi Eric", GL_AMBER);
    // gl_swap_buffer();
}

//x and y are the top left coordinates of the rectangle
void drawPaddle(int x, int y, int w, int h, color_t c) {
    //TODO: fix width/height mix-up
    //gl_draw_rect(int x, int y, int w, int h, color_t c);
    gl_draw_rect(x, y, w, h, c);
    // gl_swap_buffer();
}

void main(void) {
    test_gl();
    //TODO: fix swap buffer mess
    // gl_init(_WIDTH, _HEIGHT, GL_DOUBLEBUFFER);
    // gl_swap_buffer();
    gl_clear(gl_color(0xFF, 0xFF, 0xFF)); 
    unsigned int p_width = 100;
    unsigned int p_height = 4;
    unsigned int ball_radius = 10;
    drawPaddle(_WIDTH/5, _HEIGHT/4, p_height, p_width, GL_BLUE);
    drawPaddle(_WIDTH - _WIDTH/5 - p_width, _HEIGHT/4, p_height, p_width, GL_BLUE);
    gl_draw_ball(_WIDTH/2, _HEIGHT/2, ball_radius, GL_BLUE);
    gl_swap_buffer();
}