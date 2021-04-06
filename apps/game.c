#include "assert.h"
#include "timer.h"
#include "font.h"
#include "fb.h"
#include "gl.h"
#include "console.h"
#include "printf.h"
#include "kirby.h"
#include "string.h"
#include "shell.h"
#include <stdlib.h>
#include "i2c.h"
#include "gpio.h"
#include "keyboard.h"

#define _WIDTH 600
#define _HEIGHT 500
#define _NROWS 20
#define _NCOLS 30
#define _NPLATFORMS 10
#define _NCOINS 10
#define _TEXTCOLOR GL_RED
#define _PLATFORMCOLOR GL_BLUE
#define _KIRBYWIDTH 20
#define _COINWIDTH 13
#define _COINHEIGHT 16
#define _PLATFORMHEIGHT 16

#define _NROWS 20
#define _NCOLS 30

// --- gyro init ---
#define    MPU9250_ADDRESS            0x68 //0x6B
#define    MAG_ADDRESS                0x0C

#define    GYRO_FULL_SCALE_250_DPS    0x00
#define    GYRO_FULL_SCALE_500_DPS    0x08
#define    GYRO_FULL_SCALE_1000_DPS   0x10
#define    GYRO_FULL_SCALE_2000_DPS   0x18
#define    LSM9DS1_REGISTER_CTRL_REG6_XL 0x20
#define    LSM9DS1_REGISTER_CTRL_REG8 0x22
#define    ACC_FULL_SCALE_2_G        0x00
#define    ACC_FULL_SCALE_4_G        0x08
#define    ACC_FULL_SCALE_8_G        0x10
#define    ACC_FULL_SCALE_16_G       0x18

typedef unsigned char uint8_t;

// Write a byte (Data) in device (Address) at register (Register)
void i2c_writeByte(unsigned Address, uint8_t Register, uint8_t Data)
{
    i2c_write(MPU9250_ADDRESS,(char *)&Register,1);
    i2c_write(MPU9250_ADDRESS,(char *)&Data,1);
}

void chip_init() {
    // Set accelerometers low pass filter at 5Hz
    i2c_writeByte(MPU9250_ADDRESS,29,0x06);
    // Set gyroscope low pass filter at 5Hz
    i2c_writeByte(MPU9250_ADDRESS,26,0x06);
    
    // Configure gyroscope range
    i2c_writeByte(MPU9250_ADDRESS,27,GYRO_FULL_SCALE_1000_DPS);
    // Configure accelerometers range
    i2c_writeByte(MPU9250_ADDRESS,28,ACC_FULL_SCALE_4_G);
    // Set by pass mode for the magnetometers
    i2c_writeByte(MPU9250_ADDRESS,0x37,0x02);
    
    // Request continuous magnetometer measurements in 16 bits
    i2c_writeByte(MAG_ADDRESS,0x0A,0x16);
}

// static typedef struct {
static struct gyroType {
    int currDirection; //1 for right 2 for left
    int ax;
    int ay;
    int az;
    int oldAx;
    int gx;
    int oldGx;
}; //gyroType;

struct gyroType gyro;
// static gyroType gyro;

void readGyro() {
    unsigned char buf[1024];
    char acc_register = 0x3B;//0x80|0x28;//
    i2c_write(MPU9250_ADDRESS,&acc_register,1);
    // read accelerometer
    i2c_read(MPU9250_ADDRESS,buf,14);
    
    // convert to xyz values
    gyro.oldAx = gyro.ax;
    gyro.ax=-(buf[0]<<8 | buf[1]);
    gyro.ay=-(buf[2]<<8 | buf[3]);
    gyro.az=buf[4]<<8 | buf[5];
    gyro.oldGx = gyro.gx;
    gyro.gx = buf[8]<<8 | buf[9]; // divide by counts per degree sec (32) then take kalman
}
// --------
int initgx = 0;
struct platform {
    int width;
    int height;
    int x;
    int y;
    int draw;
};

struct coin {
    int x;
    int y;
    int draw;
};

struct platform platforms[_NPLATFORMS];
struct coin coins[_NCOINS];
int collidedCoins[_NCOINS];
int tracker = 0;
int points = 0;

// initialize window and set screen color to white
static void init_window(void) {
    i2c_init();
    gl_init(_WIDTH, _HEIGHT, GL_DOUBLEBUFFER);
    gl_clear(GL_WHITE);
    readGyro();
    initgx = gyro.gx;
    readGyro();
    initgx += gyro.gx;
    readGyro();
    initgx += gyro.gx;
    initgx /= 3;
}

// x and y coordinates are the top left corner of image
void drawKirbyStandingWalking (int x, int y, unsigned kirby_map[]) {
    int size = 1440;
    int originalx = x;
    for (int i = 0; i < size / 4; i++) {
        if (x < gl_get_width() && y < gl_get_height()) {
            gl_draw_pixel(x, y, kirby_map[i]);
        }
        if ((i+1) % _KIRBYWIDTH == 0) {
            y++;
            x = originalx;
        } else {
            x++;
        }
    }
}

void drawKirbyDropping (int x, int y, unsigned kirby_map[]) {
    int size = 1280;
    int originalx = x;
    for (int i = 0; i < size / 4; i++) {
        if (x < gl_get_width() && y < gl_get_height()) {
            gl_draw_pixel(x, y, kirby_map[i]);
        }
        if ((i+1) % _KIRBYWIDTH == 0) {
            y++;
            x = originalx;
        } else {
            x++;
        }
    }
}

void drawKirbyGhost (int x, int y, unsigned kirby_map[]) {
    int size = 1884;
    int originalx = x;
    for (int i = 0; i < size / 4; i++) {
        if (x < gl_get_width() && y < gl_get_height()) {
            gl_draw_pixel(x, y, kirby_map[i]);
        }
        if ((i+1) % 24 == 0) {
            y++;
            x = originalx;
        } else {
            x++;
        }
    }
}

//height = 70
//width = 314
//gwidth = 36
//awidth = 36
//mwidth = 60
//edwidth = 36
//space = 6
//owdith = 36
//vwidth = 36
//edwidth = 36
//rwidth = 32

// Proposed change: Create functions to draw entire letters, rather than the individual rectangles that make up these letters.
void drawGameOver() {
    //G
    gl_draw_rect(140, 162, 4, 46, GL_BLACK);
    gl_draw_rect(144, 158, 4, 4, GL_BLACK);
    gl_draw_rect(148, 154, 4, 4, GL_BLACK);
    gl_draw_rect(152, 150, 12, 4, GL_BLACK);
    gl_draw_rect(164, 154, 4, 4, GL_BLACK);
    gl_draw_rect(168, 158, 4, 4, GL_BLACK);
    gl_draw_rect(172, 162, 4, 46, GL_BLACK);
    gl_draw_rect(144, 206, 4, 6, GL_BLACK);
    gl_draw_rect(148, 210, 4, 6, GL_BLACK);
    gl_draw_rect(152, 214, 12, 6, GL_BLACK);
    gl_draw_rect(164, 210, 4, 6, GL_BLACK);
    gl_draw_rect(168, 206, 4, 6, GL_BLACK);
    gl_draw_rect(160, 178, 12, 4, GL_BLACK);
    gl_draw_rect(160, 178, 4, 8, GL_BLACK);
    //a
    gl_draw_rect(176, 186, 4, 22, GL_BLACK);
    gl_draw_rect(180, 182, 4, 4, GL_BLACK);
    gl_draw_rect(184, 178, 4, 4, GL_BLACK);
    gl_draw_rect(188, 174, 24, 4, GL_BLACK);
    gl_draw_rect(208, 174, 4, 46, GL_BLACK);
    gl_draw_rect(180, 206, 4, 6, GL_BLACK);
    gl_draw_rect(184, 210, 4, 6, GL_BLACK);
    gl_draw_rect(188, 214, 20, 6, GL_BLACK);
    gl_draw_rect(192, 190, 4, 12, GL_BLACK);
    //m
    gl_draw_rect(212, 186, 4, 34, GL_BLACK);
    gl_draw_rect(216, 182, 4, 4, GL_BLACK);
    gl_draw_rect(220, 178, 4, 4, GL_BLACK);
    gl_draw_rect(224, 174, 12, 4, GL_BLACK);
    gl_draw_rect(236, 178, 4, 4, GL_BLACK);
    gl_draw_rect(240, 182, 4, 16, GL_BLACK);
    gl_draw_rect(244, 178, 4, 4, GL_BLACK);
    gl_draw_rect(248, 174, 12, 4, GL_BLACK);
    gl_draw_rect(260, 178, 4, 4, GL_BLACK);
    gl_draw_rect(264, 182, 4, 4, GL_BLACK);
    gl_draw_rect(268, 186, 4, 34, GL_BLACK);
    gl_draw_rect(212, 214, 60, 6, GL_BLACK);
    gl_draw_rect(228, 194, 4, 20, GL_BLACK);
    gl_draw_rect(252, 194, 4, 20, GL_BLACK);
    //e
    gl_draw_rect(272, 186, 4, 22, GL_BLACK);
    gl_draw_rect(276, 182, 4, 4, GL_BLACK);
    gl_draw_rect(280, 178, 4, 4, GL_BLACK);
    gl_draw_rect(284, 174, 24, 4, GL_BLACK);
    gl_draw_rect(304, 174, 4, 46, GL_BLACK);
    gl_draw_rect(276, 206, 4, 6, GL_BLACK);
    gl_draw_rect(280, 210, 4, 6, GL_BLACK);
    gl_draw_rect(284, 214, 20, 6, GL_BLACK);
    gl_draw_rect(292, 186, 4, 4, GL_BLACK);
    gl_draw_rect(292, 198, 12, 4, GL_BLACK);
    //O
    gl_draw_rect(314, 162, 4, 46, GL_BLACK);
    gl_draw_rect(318, 158, 4, 4, GL_BLACK);
    gl_draw_rect(322, 154, 4, 4, GL_BLACK);
    gl_draw_rect(326, 150, 12, 4, GL_BLACK);
    gl_draw_rect(338, 154, 4, 4, GL_BLACK);
    gl_draw_rect(342, 158, 4, 4, GL_BLACK);
    gl_draw_rect(346, 162, 4, 46, GL_BLACK);
    gl_draw_rect(318, 206, 4, 6, GL_BLACK);
    gl_draw_rect(322, 210, 4, 6, GL_BLACK);
    gl_draw_rect(326, 214, 12, 6, GL_BLACK);
    gl_draw_rect(338, 210, 4, 6, GL_BLACK);
    gl_draw_rect(342, 206, 4, 6, GL_BLACK);
    gl_draw_rect(330, 178, 4, 12, GL_BLACK);
    //v
    gl_draw_rect(350, 174, 4, 34, GL_BLACK);
    gl_draw_rect(350, 174, 36, 4, GL_BLACK);
    gl_draw_rect(382, 174, 4, 34, GL_BLACK);
    gl_draw_rect(354, 206, 4, 6, GL_BLACK);
    gl_draw_rect(358, 210, 4, 6, GL_BLACK);
    gl_draw_rect(362, 214, 12, 6, GL_BLACK);
    gl_draw_rect(374, 210, 4, 6, GL_BLACK);
    gl_draw_rect(378, 206, 4, 6, GL_BLACK);
    gl_draw_rect(366, 174, 4, 16, GL_BLACK);
    //e
    gl_draw_rect(386, 186, 4, 22, GL_BLACK);
    gl_draw_rect(390, 182, 4, 4, GL_BLACK);
    gl_draw_rect(394, 178, 4, 4, GL_BLACK);
    gl_draw_rect(398, 174, 24, 4, GL_BLACK);
    gl_draw_rect(418, 174, 4, 46, GL_BLACK);
    gl_draw_rect(390, 206, 4, 6, GL_BLACK);
    gl_draw_rect(394, 210, 4, 6, GL_BLACK);
    gl_draw_rect(398, 214, 24, 6, GL_BLACK);
    gl_draw_rect(406, 198, 12, 4, GL_BLACK);
    gl_draw_rect(406, 186, 4, 4, GL_BLACK);
    //r
    gl_draw_rect(422, 186, 4, 34, GL_BLACK);
    gl_draw_rect(426, 182, 4, 4, GL_BLACK);
    gl_draw_rect(430, 178, 4, 4, GL_BLACK);
    gl_draw_rect(434, 174, 20, 4, GL_BLACK);
    gl_draw_rect(450, 174, 4, 18, GL_BLACK);
    gl_draw_rect(442, 186, 12, 6, GL_BLACK);
    gl_draw_rect(438, 190, 8, 6, GL_BLACK);
    gl_draw_rect(438, 190, 4, 30, GL_BLACK);
    gl_draw_rect(422, 214, 20, 6, GL_BLACK);
}

//width = 300
//height = 72
//starting x= 150
//starting y = 150
//kwidth = 36
//iwidth = 20
//rwidth = 32
//bwidth = 36
//ywdith = 36
//Dwidth = 36
//rwidth = 32
//owidth = 36
void drawKirbyDrop() {
    gl_draw_rect(150, 150, 4, 72, GL_BLACK);
    gl_draw_rect(150, 150, 36, 4, GL_BLACK);
    gl_draw_rect(166, 150, 4, 20, GL_BLACK);
    gl_draw_rect(182, 150, 4, 72, GL_BLACK);
    gl_draw_rect(174, 182, 12, 4, GL_BLACK);
    gl_draw_rect(166, 198, 4, 22, GL_BLACK);
    gl_draw_rect(150, 216, 76, 6, GL_BLACK);
    gl_draw_rect(186, 166, 20, 4, GL_BLACK);
    gl_draw_rect(186, 166, 4, 56, GL_BLACK);
    gl_draw_rect(186, 182, 20, 4, GL_BLACK);
    gl_draw_rect(202, 166, 4, 56, GL_BLACK);
    //r
    gl_draw_rect(206, 186, 4, 36, GL_BLACK);
    gl_draw_rect(210, 182, 4, 4, GL_BLACK);
    gl_draw_rect(214, 178, 4, 4, GL_BLACK);
    gl_draw_rect(218, 174, 20, 4, GL_BLACK);
    gl_draw_rect(234, 174, 4, 12, GL_BLACK);
    gl_draw_rect(226, 186, 12, 6, GL_BLACK);
    gl_draw_rect(222, 190, 8, 6, GL_BLACK);
    gl_draw_rect(222, 190, 4, 30, GL_BLACK);
    //b
    gl_draw_rect(238, 150, 20, 4, GL_BLACK);
    gl_draw_rect(238, 150, 4, 72, GL_BLACK);
    gl_draw_rect(238, 216, 24, 6, GL_BLACK);
    gl_draw_rect(254, 150, 4, 28, GL_BLACK);
    gl_draw_rect(254, 174, 8, 4, GL_BLACK);
    gl_draw_rect(262, 178, 4, 4, GL_BLACK);
    gl_draw_rect(266, 182, 4, 4, GL_BLACK);
    gl_draw_rect(270, 186, 4, 22, GL_BLACK);
    gl_draw_rect(266, 208, 4, 8, GL_BLACK);
    gl_draw_rect(262, 212, 4, 6, GL_BLACK);
    gl_draw_rect(254, 190, 4, 12, GL_BLACK);
    //y
    gl_draw_rect(274, 170, 4, 34, GL_BLACK);
    gl_draw_rect(274, 170, 36, 4, GL_BLACK);
    gl_draw_rect(290, 170, 4, 28, GL_BLACK);
    gl_draw_rect(278, 202, 4, 6, GL_BLACK);
    gl_draw_rect(282, 206, 4, 6, GL_BLACK);
    gl_draw_rect(286, 210, 8, 6, GL_BLACK);
    gl_draw_rect(290, 210, 4, 12, GL_BLACK);
    gl_draw_rect(282, 218, 12, 4, GL_BLACK);
    gl_draw_rect(282, 218, 4, 22, GL_BLACK);
    gl_draw_rect(282, 234, 16, 6, GL_BLACK);
    gl_draw_rect(298, 230, 4, 6, GL_BLACK);
    gl_draw_rect(302, 226, 4, 6, GL_BLACK);
    gl_draw_rect(306, 170, 4, 58, GL_BLACK);
    //D
    gl_draw_rect(310, 150, 24, 4, GL_BLACK);
    gl_draw_rect(310, 150, 4, 72, GL_BLACK);
    gl_draw_rect(310, 216, 24, 6, GL_BLACK);
    gl_draw_rect(334, 154, 4, 4, GL_BLACK);
    gl_draw_rect(338, 158, 4, 4, GL_BLACK);
    gl_draw_rect(342, 162, 4, 48, GL_BLACK);
    gl_draw_rect(338, 208, 4, 6, GL_BLACK);
    gl_draw_rect(334, 212, 4, 6, GL_BLACK);
    gl_draw_rect(310, 216, 24, 6, GL_BLACK);
    gl_draw_rect(326, 178, 4, 12, GL_BLACK);
    //r
    gl_draw_rect(346, 186, 4, 36, GL_BLACK);
    gl_draw_rect(350, 182, 4, 4, GL_BLACK);
    gl_draw_rect(354, 178, 4, 4, GL_BLACK);
    gl_draw_rect(358, 174, 20, 4, GL_BLACK);
    gl_draw_rect(374, 174, 4, 18, GL_BLACK);
    gl_draw_rect(366, 186, 12, 6, GL_BLACK);
    gl_draw_rect(362, 188, 8, 6, GL_BLACK);
    gl_draw_rect(362, 188, 4, 30, GL_BLACK);
    gl_draw_rect(346, 214, 20, 6, GL_BLACK);
    //o
    gl_draw_rect(378, 186, 4, 22, GL_BLACK);
    gl_draw_rect(382, 182, 4, 4, GL_BLACK);
    gl_draw_rect(386, 178, 4, 4, GL_BLACK);
    gl_draw_rect(390, 174, 12, 4, GL_BLACK);
    gl_draw_rect(402, 178, 4, 4, GL_BLACK);
    gl_draw_rect(406, 182, 4, 4, GL_BLACK);
    gl_draw_rect(410, 186, 4, 22, GL_BLACK);
    gl_draw_rect(382, 206, 4, 6, GL_BLACK);
    gl_draw_rect(386, 210, 4, 6, GL_BLACK);
    gl_draw_rect(390, 214, 12, 6, GL_BLACK);
    gl_draw_rect(402, 210, 4, 6, GL_BLACK);
    gl_draw_rect(406, 206, 4, 6, GL_BLACK);
    //p
    gl_draw_rect(414, 174, 4, 64, GL_BLACK);
    gl_draw_rect(414, 174, 24, 4, GL_BLACK);
    gl_draw_rect(414, 232, 20, 6, GL_BLACK);
    gl_draw_rect(438, 178, 4, 4, GL_BLACK);
    gl_draw_rect(442, 182, 4, 4, GL_BLACK);
    gl_draw_rect(446, 186, 4, 22, GL_BLACK);
    gl_draw_rect(442, 206, 4, 6, GL_BLACK);
    gl_draw_rect(438, 210, 4, 6, GL_BLACK);
    gl_draw_rect(430, 214, 8, 6, GL_BLACK);
    gl_draw_rect(430, 214, 4, 22, GL_BLACK);
    gl_draw_rect(430, 190, 4, 12, GL_BLACK);
}

void welcome() {
    
    char* strWelcome = "Welcome to Kirby Drop!";
    int lengthWelcome = strlen(strWelcome);
    int x1 = _WIDTH / 2 - lengthWelcome * gl_get_char_width() / 2;
    int y1 = _HEIGHT / 4;
    char* str = "Swipe right to start";
    int length = strlen(str);
    int x = _WIDTH / 2 - length * gl_get_char_width() / 2;
    int y = _HEIGHT / 2;
    
    gl_clear(GL_PASTEL_BLUE);
    drawKirbyDrop();
    drawKirbyStandingWalking(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 9) * 5, blue_kirby_map);
    gl_draw_string(x, y, str, _TEXTCOLOR);
    gl_swap_buffer();
    timer_delay(2);
    // detect remote control sensor to start
    while (true) {
        readGyro();
        if (gyro.ax < -45000 && gyro.ax > -65000) {
            break;
        }
    }
    gl_swap_buffer();
}

// draw singular platform
void drawPlatform(int x, int y, int width, int height, color_t c) {
    //x and y are the top left coordinates of the rectangle
    int size = sizeof(brick_map);
    int originalx = x;
    int originaly = y;
    
    int numBricks = width / _PLATFORMHEIGHT;
    for (int n = 0; n < numBricks; n++) {
        // draws single brick
        for (int i = 0; i < size / 4; i++) {
            if (x < gl_get_width() && y < gl_get_height()) {
                gl_draw_pixel(x, y, brick_map[i]);
            }
            if ((i+1) % _PLATFORMHEIGHT == 0) {
                y++;
                x = originalx;
            } else {
                x++;
            }
        }
        originalx += _PLATFORMHEIGHT;
        x = originalx;
        y = originaly;
    }
}

 // draw singular coin
void drawCoin(int x, int y) {
    //x and y are the top left coordinates of the coin
    int size = sizeof(coin_map);
    int originalx = x;
    for (int i = 0; i < size / 4; i++) {
        if (x < gl_get_width() && y < gl_get_height()) {
            gl_draw_pixel(x, y, coin_map[i]);
        }
        if ((i+1) % _COINWIDTH == 0) {
            y++;
            x = originalx;
        } else {
            x++;
        }
    }
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
    unsigned int min_p_width = _KIRBYWIDTH * 4;
    unsigned int max_p_width = _WIDTH / 2;
    unsigned int max_x_coord = _WIDTH - _KIRBYWIDTH * 4;
    p.height = 4;
    p.y = 0;
    p.width = min_p_width + rand() % (max_p_width + 1 - min_p_width);
    p.x = rand() % (max_x_coord - 1);
    return p;
}

//generate and return a single random coin
struct coin generateRandomCoin(int min_x_coord, int max_x_coord) {
    //random number generator seed
    int t = timer_get_ticks() % 1024;
    for (int i = 0; i < t; i++) {
        rand();
    }
    struct coin c;
    c.y = 0;
    c.x = min_x_coord + rand() % (max_x_coord + 1 - min_x_coord);
    c.draw = 1;
    return c;
}

// generate initial array of platforms
void generatePlatformsandCoins() {
    int doubleCoins = 0;
    struct platform p;
    struct coin c;
    int y_coord = 0;
    for (int i = 0; i < _NPLATFORMS; i++) {
        p = generateRandomPlatform();
        p.y = y_coord;
        platforms[i] = p;
        if (doubleCoins == 1) {
            c = generateRandomCoin(p.x, p.x + p.width - _COINWIDTH*2);
            c.y = y_coord - _COINHEIGHT;
            coins[i-1] = c;
            c = generateRandomCoin(c.x, p.x + p.width - _COINWIDTH);
            c.y = y_coord - _COINHEIGHT;
            coins[i] = c;
            doubleCoins = 0;
        } else if (p.width % 3 == 0) {
            doubleCoins = 1;
        } else {
            c = generateRandomCoin(p.x, p.x + p.width - _COINWIDTH);
            c.y = y_coord - _COINHEIGHT;
            coins[i] = c;
        }
        y_coord += ( _HEIGHT / _NPLATFORMS);
    }
}

// checks if Kirby collides with a platform
int checkCollide(int x, int y, unsigned kirby_map[]) {
    int h = _KIRBYWIDTH - 1;
    for (int i = 0; i < _KIRBYWIDTH; i++) {
        if (gl_read_pixel(x + i, y + h) == GL_TAN || gl_read_pixel(x + i, y + h) == GL_DARK_BROWN || gl_read_pixel(x + i, y + h - 1) == GL_TAN || gl_read_pixel(x + i, y + h - 1) == GL_DARK_BROWN || gl_read_pixel(x + i, y + h - 2) == GL_TAN || gl_read_pixel(x + i, y + h - 2) == GL_DARK_BROWN) {
            return 0;
        }
    }
    return 1;
}

int checkCoinIndex(int x, int y) {
    for (int i = 0; i < _NCOINS; i++) {
        if ((x >= coins[i].x) && (x <= (coins[i].x + _COINWIDTH)) && (y >= coins[i].y) && (y <= (coins[i].y + _COINHEIGHT))) {
            coins[i].draw = 0;
            return i;
        }
    }
}

// checks if Kirby collides with a coin
// Proposed change: Can definitely be optimized
int checkCoin(int x, int y, unsigned kirby_map[]) {
    int h;
    if (kirby_map == dropping_kirby_left_map || dropping_kirby_right_map) {
        h = _KIRBYWIDTH * 9 / 10;
    } else {
        h = _KIRBYWIDTH;
    }
    for (int i = 0; i < h; i++) {
        if (gl_read_pixel(x, y + i) == GL_ORANGE || gl_read_pixel(x, y + i) == GL_AMBER) {
            return checkCoinIndex(x, y + i);
        }
    }
    for (int i = 0; i < h; i++) {
        if (gl_read_pixel(x + _KIRBYWIDTH - 1, y + i) == GL_ORANGE || gl_read_pixel(x + _KIRBYWIDTH - 1, y + i) == GL_AMBER) {
            return checkCoinIndex(x + _KIRBYWIDTH - 1, y + i);
        }
    }
    for (int i = 0; i < _KIRBYWIDTH; i++) {
        if (gl_read_pixel(x + i, y + h) == GL_ORANGE || gl_read_pixel(x + i, y + h) == GL_AMBER) {
            return checkCoinIndex(x + i, y + h);
        }
    }
    return -1;
}

// move Kirby and platforms on display
void scrollScreen() {
    int x_coord = _WIDTH / 2 - _KIRBYWIDTH / 2;
    int y_coord = _KIRBYWIDTH;
    int coinIndex;
    char printPoints[5];
    char direction;
    int skipdraw = 0;
    int justAdded = 0;
    platforms[0].draw = 1;
    platforms[1].draw = 1;
    
    // Proposed change: Huge messy block of if else statements - can definitely break this down into more organized logic
    struct platform p;
    struct coin c;
    // loop to animate both platforms and Kirby
    for (int i = 0; i < _HEIGHT * 5; i++) {
        unsigned int * previous_kirby = dropping_kirby_right_map;
        coinIndex = checkCoin(x_coord, y_coord, previous_kirby);
        if (justAdded != 1) {
            if (coinIndex >= 0) {
                points++;
                justAdded = 1;
            }
        } else {
            justAdded = 0;
        }
        gl_clear(GL_WHITE);
        // if new platform and coin should be generated
        if (i % (_HEIGHT / _NPLATFORMS) == 0) {
            p = generateRandomPlatform();
            p.y = _HEIGHT;
            c = generateRandomCoin(p.x, p.x + p.width - _COINWIDTH);
            c.y = p.y - _COINHEIGHT;
            // shift circular arrays of platforms and coins
            for (int j = 0; j < _NPLATFORMS - 1; j++) {
                platforms[j] = platforms[j + 1];
                coins[j] = coins[j+1];
            }
            platforms[_NPLATFORMS - 1] = p;
            coins[_NCOINS - 1] = c;
        }
        // move platforms and coins
        for (int i = 0; i < _NPLATFORMS; i++) {
            skipdraw = 0;
            platforms[i].y--;
            if (platforms[i].draw == 1) {
                platforms[i].draw == 0;
            } else {
                drawPlatform(platforms[i].x, platforms[i].y, platforms[i].width, platforms[i].height, _PLATFORMCOLOR);
            }
            coins[i].y--;
            if (coins[i].draw == 1) {
                drawCoin(coins[i].x, coins[i].y);
            }
        }
        // if Kirby collides with platform
        if (checkCollide(x_coord, y_coord, previous_kirby) == 0) {
            if (x_coord <= 0 || (x_coord + _KIRBYWIDTH) >= _WIDTH) {
                if (gyro.currDirection == 1) {
                    y_coord -= 1;
                    x_coord -= 3;
                    drawKirbyStandingWalking(x_coord, y_coord, standing_kirby_right_map);
                    previous_kirby = standing_kirby_right_map;
                    gyro.currDirection = 2;
                } else {
                    y_coord -= 1;
                    x_coord += 3;
                    drawKirbyStandingWalking(x_coord, y_coord, standing_kirby_left_map);
                    previous_kirby = standing_kirby_left_map;
                    gyro.currDirection = 1;
                }
            }
            else {
                readGyro();
                if (gyro.ax < -45000 && gyro.ax > -65000) {
//                        printf("Moving right \n");
                    gyro.currDirection = 1;
                    x_coord += 3;
                    y_coord -= 1;
                    if (y_coord % 2 == 0) {
                        drawKirbyStandingWalking(x_coord, y_coord, standing_kirby_right_map);
                        previous_kirby = standing_kirby_right_map;
                    } else {
                        drawKirbyStandingWalking(x_coord, y_coord, walking_kirby_right_map);
                        previous_kirby = walking_kirby_right_map;
                    }
                } else if (gyro.ax < -14000 && gyro.ax > -34000) {
                    gyro.currDirection = 2;
//                       printf("Moving left \n");
                    x_coord -= 3;
                    y_coord -= 1;
                    if (y_coord % 2 == 0) {
                        drawKirbyStandingWalking(x_coord, y_coord, standing_kirby_left_map);
                        previous_kirby = standing_kirby_left_map;
                    } else {
                        drawKirbyStandingWalking(x_coord, y_coord, walking_kirby_left_map);
                        previous_kirby = walking_kirby_left_map;
                    }
                } else {
//                    printf("no movement\n");
                    if (gyro.currDirection == 1) {
                        // move right
                        x_coord += 3;
                        y_coord -= 1;
                        if (y_coord % 2 == 0) {
                            drawKirbyStandingWalking(x_coord, y_coord, standing_kirby_right_map);
                            previous_kirby = standing_kirby_right_map;
                        } else {
                            drawKirbyStandingWalking(x_coord, y_coord, walking_kirby_right_map);
                            previous_kirby = walking_kirby_right_map;
                        }
                    } else if (gyro.currDirection == 2){
                        //move left
                        x_coord -= 3;
                        y_coord -= 1;
                        if (y_coord % 2 == 0) {
                            drawKirbyStandingWalking(x_coord, y_coord, standing_kirby_left_map);
                            previous_kirby = standing_kirby_left_map;
                        } else {
                            drawKirbyStandingWalking(x_coord, y_coord, walking_kirby_left_map);
                            previous_kirby = walking_kirby_left_map;
                        }
                    }
                }
            }
        } else {
            if (x_coord <= 0 || (x_coord + _KIRBYWIDTH) >= _WIDTH) {
                if (gyro.currDirection == 1) {
                    x_coord -= 3;
                    y_coord += 3;
                    drawKirbyDropping(x_coord, y_coord, dropping_kirby_right_map);
                    previous_kirby = dropping_kirby_right_map;
                    gyro.currDirection == 2;
                } else {
                    x_coord += 3;
                    y_coord += 3;
                    drawKirbyDropping(x_coord, y_coord, dropping_kirby_left_map);
                    previous_kirby = dropping_kirby_left_map;
                    gyro.currDirection == 1;
                }
            } else {
                readGyro();
                if (gyro.ax < -45000 && gyro.ax > -65000) {
//                    printf("Moving right \n");
                    x_coord += 3;
                    y_coord+=3;
                    drawKirbyDropping(x_coord, y_coord, dropping_kirby_right_map);
                    previous_kirby = dropping_kirby_right_map;
                    gyro.currDirection = 1;
                } else if (gyro.ax < -14000 && gyro.ax > -34000){
//                    printf("Moving left \n");
                    x_coord -= 3;
                    y_coord+=3;
                    drawKirbyDropping(x_coord, y_coord, dropping_kirby_left_map);
                    previous_kirby = dropping_kirby_left_map;
                    gyro.currDirection = 2;
                } else {
//                    printf("no movement\n");
                    if (gyro.currDirection == 1) {
                        // move right
                        x_coord += 3;
                        y_coord+=3;
                        drawKirbyDropping(x_coord, y_coord, dropping_kirby_right_map);
                        previous_kirby = dropping_kirby_right_map;
                    } else if (gyro.currDirection == 2){
                        //move left
                        x_coord -= 3;
                        y_coord+=3;
                        drawKirbyDropping(x_coord, y_coord, dropping_kirby_left_map);
                        previous_kirby = dropping_kirby_left_map;
                    }
                }
            }
        }
        // Kirby hits top or bottom edge of screen
        if (y_coord + _KIRBYWIDTH >= _HEIGHT || y_coord <= 0) {
            timer_delay(1);
            break;
        }
        drawCoin(5, _HEIGHT - _COINHEIGHT - 5);
        gl_draw_string(5 + _COINWIDTH + 2, _HEIGHT - _COINHEIGHT - 3, "x", GL_RED);
        snprintf(printPoints, 5, "%d", points);
        gl_draw_string(5 + _COINWIDTH * 2 + 2, _HEIGHT - _COINHEIGHT - 3, printPoints, GL_RED);
        gl_swap_buffer();
        timer_delay_ms(10);
    }
}

void drawScoreBoard() {
    gl_draw_rect(100, 150, 400, 200, GL_TAN);
    gl_draw_rect(95, 145, 410, 210, GL_DARK_BROWN);
}

// Screen that displays game over information and points scored  
int gameOver() {
    char* strAgain = "Swipe right to play again!";
    int lengthAgain = strlen(strAgain);
    int x2 = _WIDTH / 2 - lengthAgain * gl_get_char_width() / 2;
    int y2 = _HEIGHT / 2;
    char printPoints[5];

    gl_swap_buffer();
    gl_clear(GL_WHITE);
    drawGameOver();
    drawCoin(276, _HEIGHT / 8 * 5);
    gl_draw_string(293, _HEIGHT / 8 * 5 + 2, "x", GL_RED);
    snprintf(printPoints, 5, "%d", points);
    gl_draw_string(309, _HEIGHT / 8 * 5 + 2, printPoints, GL_RED);
    drawKirbyGhost(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 4) * 3, ghost_kirby_map);
    gl_draw_string(x2, y2, strAgain, GL_RED);
    gl_swap_buffer();
    timer_delay(2);
    
    while (true) {
        readGyro();
        if (gyro.ax < -45000 && gyro.ax > -65000) {
            break;
        }
    }
    
    points = 0;
    gl_swap_buffer();
    gl_clear(GL_WHITE);
    return 1;
}

// main program
void main(void) {
    printf("char width = %d", gl_get_char_width());
    keyboard_init();
    interrupts_global_enable(); // everything fully initialized, now turn on interrupts
    init_window();
    welcome();
    int keepPlaying = 1;
    // keep playing as many times as user wants
    while (keepPlaying == 1) {
        gl_swap_buffer();
        gl_clear(GL_WHITE);
        generatePlatformsandCoins();
        scrollScreen();
        keepPlaying = gameOver();
    }
    gl_swap_buffer();
    
//    flipArray();
//    readPixelArray();
}

// mirror image flip array by printing the elements of each line in reverse order
void flipArray() {
    int counter = 0;
    int start = 0;
    int end = _KIRBYWIDTH - 1;
    unsigned map[1440];
    memcpy(map, standing_kirby_right_map, 1440);
    while (end <= 1440) {
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
    for (int i = 0; i < 1440; i++) {
        printf("0x%x, ", map[i]);
    }
}

// convert background format
//void readPixelArray() {
//    int size = sizeof(kirby_background_map);
//    unsigned map[size / 4];
//    int counter = 0;
//    for (int i = 0; i < size; i += 4) {
//        unsigned pixel = pixel | (kirby_background_map[i + 3] << 24);
//        pixel = pixel | (kirby_background_map[i + 2] << 16);
//        pixel = pixel | (kirby_background_map[i + 1] << 8);
//        pixel = pixel | (kirby_background_map[i]);
//        map[counter] = pixel;
//        counter++;
//    }
//
//    size = sizeof(map);
//
//    printf("size = %d", size);
//    int x = 0;
//    int y = 0;
//    for (int i = 0; i < size; i++) {
//        if (x < gl_get_width() && y < gl_get_height()) {
//            gl_draw_pixel(x, y, map[i]);
//        }
//        if ((i+1) % 500 == 0) {
//            y++;
//            x = 0;
//        } else {
//            x++;
//        }
//    }
//}

