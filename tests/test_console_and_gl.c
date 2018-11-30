#include "assert.h"
#include "timer.h"
#include "fb.h"
#include "gl.h"
#include "console.h"
#include "printf.h"
#include <stdlib.h>

#define _WIDTH 600
#define _HEIGHT 500
#define _NROWS 20
#define _NCOLS 30
#define _BALLRADIUS 10

static struct platform {
    int width;
    int height;
    int x;
    int y;
};

struct platform platforms[10];

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
    //x and y are the top left coordinates of the rectangle
    gl_draw_rect(x, y, w, h, c);
}

// generate and return a single random platform
struct platform generateRandomPlatform() {
    //random number generator seed
    int t = timer_get_ticks() % 1024;
    for (int i = 0; i < t; i++) {
        rand();
    }
    // fix height of platform and vertical gap between each platform
    // randomize x coordinate and length of each platform (set min and max)
    struct platform p;
    unsigned int min_p_width = _BALLRADIUS * 4;
    unsigned int max_p_width = _WIDTH / 2;
    unsigned int max_x_coord = _WIDTH - _BALLRADIUS * 4;
    p.height = 4;
    p.y = 0;
    p.width = min_p_width + rand() % (max_p_width + 1 - min_p_width);
    p.x = rand() % (max_x_coord - 1);
    return p;
}

// generate initial array of platforms
void generatePlatforms() {
    struct platform p;
    int y_coord = 0;
    unsigned int num_platforms = 10;
    for (int i = 0; i < num_platforms; i++) {
        p = generateRandomPlatform();
        y_coord += ( _HEIGHT / num_platforms);
        p.y = y_coord;
//        printf("p.x = %d\n", p.x);
//        printf("p.y = %d\n", p.y);
//        printf("p.width = %d\n", p.width);
//        printf("p.height = %d\n", p.height);
        drawPlatform(p.x, p.y, p.height, p.width, GL_BLUE);
        platforms[i] = p;
    }
}

// move ball on display
void moveBall() {
    // draw ball frame by frame
    // swap buffer each time
    int x_coord = _WIDTH/2;
    int y_coord = _BALLRADIUS;
    for (int i = 0; i < _HEIGHT-(2*_BALLRADIUS); i++) {
        gl_draw_ball(x_coord, y_coord, _BALLRADIUS, GL_WHITE);
        y_coord++;
        if (gl_draw_ball(x_coord, y_coord, _BALLRADIUS, GL_BLUE) == 0){
            gl_draw_ball(x_coord, y_coord, _BALLRADIUS, GL_WHITE);
            gl_draw_ball(x_coord, y_coord-1, _BALLRADIUS, GL_BLUE);
            break;
        }
        gl_draw_ball(x_coord, y_coord, _BALLRADIUS, GL_BLUE);
        gl_swap_buffer();
        timer_delay_ms(100);
        gl_swap_buffer();
    }
}

void movePlatforms() {
    struct platform p;
    int start = 0;
    // iterate through a circular array, generating a new random platform each time
    for (int i = start; i < start + 10; i++) {
        int index = i % 10;
        p = generateRandomPlatform();
        p.y = _HEIGHT;
        platforms[index] = p;
        start++;
    }
}

// main program
void main(void) {
    init_window();
    drawBall();
    generatePlatforms();
    moveBall();
//    movePlatforms();
    gl_swap_buffer();
}











// possible tests
// gl_draw_pixel(_WIDTH/3, _HEIGHT/3, GL_AMBER);
// assert(gl_read_pixel(_WIDTH/3, _HEIGHT/3) == GL_AMBER);
// gl_draw_rect(_WIDTH/2 - 20, _HEIGHT/2 - 20, 4, 50, GL_BLUE);
// gl_draw_char(60, 10, 'A', GL_BLUE);
// gl_draw_string(300, 300, "Hi Eric", GL_AMBER);
// gl_swap_buffer();
