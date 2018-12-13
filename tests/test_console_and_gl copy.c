#include "assert.h"
#include "timer.h"
#include "fb.h"
#include "gl.h"
#include "console.h"
#include "printf.h"
#include "kirby.h"
#include "kirby_background.h"
#include "string.h"
#include <stdlib.h>

#define _WIDTH 600
#define _HEIGHT 500
#define _NROWS 20
#define _NCOLS 30
#define _NPLATFORMS 10
#define _TEXTCOLOR GL_BLUE
#define _PLATFORMCOLOR GL_BLUE
#define _KIRBYWIDTH 20

#define _NROWS 20
#define _NCOLS 30

static struct platform {
    int width;
    int height;
    int x;
    int y;
};

struct platform platforms[_NPLATFORMS];

// initialize window and set screen color to white
static void init_window(void) {
    gl_init(_WIDTH, _HEIGHT, GL_DOUBLEBUFFER);
    gl_clear(GL_WHITE);
}

// x and y coordinates are the top left corner of image
int drawKirby (int x, int y, unsigned *kirby_map) {
    int size = sizeof(standing_kirby_left_map);
    int originalx = x;
    for (int i = 0; i < size / 4; i++) {
        if (gl_read_pixel(x, y) == _PLATFORMCOLOR) {
            return 0;
        }
        if (x < gl_get_width() && y < gl_get_height()) {
            gl_draw_pixel(x, y, standing_kirby_left_map[i]);
        }
        if ((i+1) % _KIRBYWIDTH == 0) {
            y++;
            x = originalx;
        } else {
            x++;
        }
    }
    return 1;
}

void welcome() {
    char* strWelcome = "Welcome to Kirby Drop!";
    int lengthWelcome = strlen(strWelcome);
    int x1 = _WIDTH / 2 - lengthWelcome * gl_get_char_width() / 2;
    int y1 = _HEIGHT / 4;
    char* str = "Press 'A' to Start";
    int length = strlen(str);
    int x = _WIDTH / 2 - length * gl_get_char_width() / 2;
    int y = _HEIGHT / 2;
    
    gl_draw_string(x1, y1, strWelcome, _TEXTCOLOR);
    drawKirby(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 8) * 3, standing_kirby_right_map);
    timer_delay(2);

    // loop until user presses Start
    for (int i = 0; i < 4; i++) {
        gl_draw_string(x1, y1, strWelcome, _TEXTCOLOR);
        drawKirby(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 8) * 3, standing_kirby_right_map);
        gl_draw_string(x, y, str, _TEXTCOLOR);
        timer_delay(1);
        gl_swap_buffer();
        gl_clear(GL_WHITE);
//        gl_draw_string(x1, y1, strWelcome, _TEXTCOLOR);
//        drawKirby(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 8) * 3, standing_kirby_right_map);
//        gl_draw_string(x, y, str, GL_WHITE);
//        timer_delay(1);
//        gl_swap_buffer();
//        gl_clear(GL_WHITE);
    }
    gl_swap_buffer();
}

// draw singular platform
void drawPlatform(int x, int y, int w, int h, color_t c) {
    //x and y are the top left coordinates of the rectangle
    gl_draw_rect(x, y, w, h, c);
}

// generate and return a single random platform
// consider making platforms more attractive?
struct platform generateRandomPlatform() {
    //random number generator seed
    int t = timer_get_ticks() % 1024;
    for (int i = 0; i < t; i++) {
        rand();
    }
    // fix height of platform and vertical gap between each platform
    // randomize x coordinate and length of each platform (set min and max)
    struct platform p;
    unsigned int min_p_width = _KIRBYWIDTH * 4;
    unsigned int max_p_width = _WIDTH / 2;
    unsigned int max_x_coord = _WIDTH - _KIRBYWIDTH * 4;
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
    for (int i = 0; i < _NPLATFORMS; i++) {
        p = generateRandomPlatform();
        p.y = y_coord;
        platforms[i] = p;
        y_coord += ( _HEIGHT / _NPLATFORMS);
    }
}

// move ball and platforms on display
void scrollScreen() {
    // starting coordinates of kirby
    int x_coord = _WIDTH / 2 - _KIRBYWIDTH / 2;
    int y_coord = _KIRBYWIDTH;
    
    struct platform p;
    
    // loop to animate both platforms and ball
    for (int i = 0; i < _HEIGHT * 5; i++) {
        gl_clear(GL_WHITE);
        // if new platform should be generated
        if (i % (_HEIGHT / _NPLATFORMS) == 0) {
            p = generateRandomPlatform();
            p.y = _HEIGHT;
            // shift circular array
            for (int j = 0; j < _NPLATFORMS - 1; j++) {
                platforms[j] = platforms[j + 1];
            }
            platforms[_NPLATFORMS - 1] = p;
        }
        // move platforms
        for (int i = 0; i < _NPLATFORMS; i++) {
            platforms[i].y--;
            drawPlatform(platforms[i].x, platforms[i].y, platforms[i].width, platforms[i].height, _PLATFORMCOLOR);
        }
        // if Kirby collides with platform
        if (drawKirby(x_coord, y_coord, standing_kirby_left_map) == 0) {
            // if not touching left edge of screen
            if (x_coord > 0) {
                // moves ball to the left
                x_coord -= 1;
            }
            // moves Kirby up with platform
            y_coord -= 1;
            drawKirby(x_coord, y_coord, standing_kirby_left_map);
        } else {
            // ball continues free-falling
            y_coord++;
            drawKirby(x_coord, y_coord, standing_kirby_left_map);
        }
        // Kirby hits top or bottom edge of screen
        if (y_coord + _KIRBYWIDTH == _HEIGHT || y_coord == 0) {
            timer_delay(2);
            break;
        }
        gl_swap_buffer();
        timer_delay_ms(10);
    }
    gameOver();
}

int gameOver() {
    char* strEnd = "Game Over!";
    int lengthEnd = strlen(strEnd);
    int x1 = _WIDTH / 2 - lengthEnd * gl_get_char_width() / 2;
    int y1 = _HEIGHT * (1 / 4);
    char* strAgain = "Press 'A' to Play Again";
    int lengthAgain = strlen(strAgain);
    int x2 = _WIDTH / 2 - lengthAgain * gl_get_char_width() / 2;
    int y2 = _HEIGHT * (2 / 4);
    char* strExit = "Press 'B' to Exit";
    int lengthExit = strlen(strExit);
    int x3 = _WIDTH / 2 - lengthExit * gl_get_char_width() / 2;
    int y3 = _HEIGHT * (3 / 4);
    
    gl_swap_buffer();
    gl_clear(GL_WHITE);
    gl_draw_string(x1, y1, strEnd, _TEXTCOLOR);
    drawKirby(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 8) * 3, standing_kirby_right_map);
    timer_delay(2);
    
    // loop until user presses button
    for (int i = 0; i < 4; i++) {
        gl_draw_string(x1, y1, strEnd, _TEXTCOLOR);
        drawKirby(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 8) * 3, standing_kirby_right_map);
        gl_draw_string(x2, y2, strAgain, _TEXTCOLOR);
        gl_draw_string(x3, y3, strExit, _TEXTCOLOR);
        timer_delay(1);
        gl_swap_buffer();
        gl_clear(GL_WHITE);
        
        gl_draw_string(x1, y1, strEnd, GL_WHITE);
        drawKirby(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 8) * 3, standing_kirby_right_map);
        gl_draw_string(x2, y2, strAgain, GL_WHITE);
        gl_draw_string(x3, y3, strExit, GL_WHITE);
        timer_delay(1);
        gl_swap_buffer();
        gl_clear(GL_WHITE);
    }
    gl_swap_buffer();
    return 0;
}

// main program
void main(void) {
    init_window();
    welcome();
    int keepPlaying = 1;
    // keep playing as many times as user wants
//    while (keepPlaying == 1) {
        generatePlatforms();
        scrollScreen();
        keepPlaying = gameOver();
//    }
    gl_swap_buffer();
//    flipArray();
//    readPixelArray();
}

// mirror image flip array by printing the elements of each line in reverse order
void flipArray() {
    int counter = 0;
    int start = 0;
    int end = _KIRBYWIDTH - 1;
    unsigned map[1436];
    memcpy(map, standing_kirby_right_map, 1436);
    while (end <= 1436) {
        // reverses each line
        while (start < end) {
            int temp = map[start];
            map[start] = map[end];
            map[end] = temp;
            start++;
            end--;
            counter++;
        }
        start += counter;
        end += counter * 3;
        counter = 0;
    }
    for (int i = 0; i < 1436; i++) {
        printf("0x%x, ", map[i]);
    }
}

// convert background format
void readPixelArray() {
    int size = sizeof(kirby_background_map);
    unsigned map[size / 4];
    int counter = 0;
    for (int i = 0; i < size; i += 4) {
        unsigned pixel = pixel | (kirby_background_map[i + 3] << 24);
        pixel = pixel | (kirby_background_map[i + 2] << 16);
        pixel = pixel | (kirby_background_map[i + 1] << 8);
        pixel = pixel | (kirby_background_map[i]);
        map[counter] = pixel;
        counter++;
    }
    for (int i = 0; i < counter; i++) {
        printf("%x, ", map[i]);
    }
    
    int size = sizeof(kirby_background_map);
    int originalx = x;
    for (int i = 0; i < size / 4; i++) {
        if (gl_read_pixel(x, y) == _PLATFORMCOLOR) {
            return 0;
        }
        if (x < gl_get_width() && y < gl_get_height()) {
            gl_draw_pixel(x, y, kirby_background_map[i]);
        }
        if ((i+1) % 500 == 0) {
            y++;
            x = originalx;
        } else {
            x++;
        }
    }
    return 1;
}

