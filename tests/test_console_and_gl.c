#include "assert.h"
#include "timer.h"
#include "fb.h"
#include "gl.h"
#include "console.h"
#include "printf.h"

// added libraries
#include <stdlib.h>

#define _WIDTH 600
#define _HEIGHT 500

#define _NROWS 20
#define _NCOLS 30

#define _BALLRADIUS 10

// initialize window
static void init_window(void) {
    gl_init(_WIDTH, _HEIGHT, GL_DOUBLEBUFFER);
    gl_clear(gl_color(0xFF, 0xFF, 0xFF));
}

// draw and fill ball
void drawBall() {
    gl_draw_ball(_WIDTH/2, 0 + _BALLRADIUS, _BALLRADIUS, GL_BLUE);
}

// draw singular platform
void drawPlatform(int x, int y, int w, int h, color_t c) {
    //TODO: fix width/height mix-up
    //x and y are the top left coordinates of the rectangle
    gl_draw_rect(x, y, w, h, c);
}

// randomly generate platforms on screen
void drawAllPlatforms() {
    // fix height of platform and vertical gap between each platform
    // randomize x coordinate and length of each platform (set min and max)
    
    unsigned int p_height = 4;
    unsigned int p_width = 0;
    unsigned int x_coord = 0;
    unsigned int y_coord = 0;
    unsigned int num_platforms = 10;
    
    unsigned int min_p_width = _BALLRADIUS * 4;
    unsigned int max_p_width = _WIDTH / 2;
    unsigned int max_x_coord = _WIDTH - _BALLRADIUS * 4;
    
    for (int i = 0; i < num_platforms; i++) {
        p_width = min_p_width + rand() % (max_p_width + 1 - min_p_width);
        x_coord = rand() % (max_x_coord - 1);
        y_coord += (_HEIGHT / num_platforms);
        drawPlatform(x_coord, y_coord, p_height, p_width, GL_BLUE);
    }
}

//// move ball on display
//void moveBall() {
//    // draw ball frame by frame
//    // clear and swap buffer each time
//    int x_coord = _WIDTH/2;
//    int y_coord = 0 + _BALLRADIUS;
//    for (int i = 0; i < _HEIGHT; i++) {
//        gl_draw_ball(x_coord, y_coord, _BALLRADIUS, GL_WHITE);
//        y_coord++;
////        gl_clear(gl_color(0xFF, 0xFF, 0xFF));
//        gl_draw_ball(x_coord, y_coord, _BALLRADIUS, GL_BLUE);
////        timer_delay(1);
////        gl_swap_buffer();
//    }
//}

// main program
void main(void) {
    init_window();
    
    drawBall();
    drawAllPlatforms();
    
//    moveBall();
    
    //TODO: fix swap buffer mess
    gl_swap_buffer();
}











// possible tests
// gl_draw_pixel(_WIDTH/3, _HEIGHT/3, GL_AMBER);
// assert(gl_read_pixel(_WIDTH/3, _HEIGHT/3) == GL_AMBER);
// gl_draw_rect(_WIDTH/2 - 20, _HEIGHT/2 - 20, 4, 50, GL_BLUE);
// gl_draw_char(60, 10, 'A', GL_BLUE);
// gl_draw_string(300, 300, "Hi Eric", GL_AMBER);
// gl_swap_buffer();
