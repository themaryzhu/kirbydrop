#include "assert.h"
#include "timer.h"
#include "fb.h"
#include "gl.h"
#include "console.h"
#include "printf.h"

/* Note that to use the console, one should only have to
 * call console_init. To use the graphics library, one
 * should only have to call gl_init. If your main() requires
 * more than this, then your code will not pass tests and
 * will likely have many points deducted. Our GL tests
 * will call gl_init then invoke operations; our console
 * tests will call console_init then invoke operations.
 * To guarantee that tests will pass, make sure to also
 * run tests for each component separately.
 */

#define _WIDTH 600
#define _HEIGHT 500

#define _NROWS 20
#define _NCOLS 30


static void test_gl(void)
{
    gl_init(_WIDTH, _HEIGHT, GL_DOUBLEBUFFER);

    gl_clear(gl_color(0xFF, 0, 0xFF)); // Background should be purple.

    // Draw an amber pixel at an arbitrary spot.
    gl_draw_pixel(_WIDTH/3, _HEIGHT/3, GL_AMBER);
    assert(gl_read_pixel(_WIDTH/3, _HEIGHT/3) == GL_AMBER);

//    // Basic rectangle should be blue in center of screen
    gl_draw_rect(_WIDTH/2 - 20, _HEIGHT/2 - 20, 40, 40, GL_BLUE);
//
//    // Should write a single character
    gl_draw_char(60, 10, 'A', GL_BLUE);

//    gl_swap_buffer();
    
    gl_draw_string(300, 300, "Hi Eric", GL_AMBER);
    
    gl_swap_buffer();
}

static void test_gl_str(void) {
    gl_init(_WIDTH, _HEIGHT, GL_DOUBLEBUFFER);
    
    gl_clear(gl_color(0xFF, 0, 0xFF)); // Background should be purple.
    
    gl_draw_string(60, 10, "Hello", GL_BLUE);
    
    gl_draw_string(630, 10, "Stanford", GL_BLUE); // offscreen
}

void main(void)
{
    test_gl();
//    test_gl_str();

    /* TODO: Add tests here to test your graphics library and console.
       For the framebuffer and graphics libraries, make sure to test
       single & double buffering and drawing/writing off the right or
       bottom edge of the frame buffer.
       For the console, make sure to test wrap-around and scrolling.
    */
}
