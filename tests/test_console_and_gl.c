#include "assert.h"
#include "timer.h"
#include "fb.h"
#include "gl.h"
#include "console.h"
#include "printf.h"
#include "kirby.h"
#include <stdlib.h>
#include "strings.h"
#include "i2c.h"
#include "gpio.h"

#define _WIDTH 600
#define _HEIGHT 500
#define _NROWS 20
#define _NCOLS 30
#define _NPLATFORMS 10
#define _BALLRADIUS 10
#define _BALLCOLOR GL_BLUE
#define _PLATFORMCOLOR GL_BLUE
#define _KIRBYWIDTH 20

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
    i2c_init();
    unsigned char buf[1024];
    // while (1) {
    char acc_register = 0x3B;//0x80|0x28;//
    i2c_write(MPU9250_ADDRESS,&acc_register,1);
    // read accelerometer
    i2c_read(MPU9250_ADDRESS,buf,14); 

    // printf("%d\n", buf);
    // convert to xyz values 
    gyro.oldAx = gyro.ax;
    gyro.ax=-(buf[0]<<8 | buf[1]);
    gyro.ay=-(buf[2]<<8 | buf[3]);
    gyro.az=buf[4]<<8 | buf[5];
    // printf("%d,%d,%d\n",gyro.ax,gyro.ay,gyro.az);
    gyro.oldGx = gyro.gx;
    gyro.gx = buf[8]<<8 | buf[9]; // divide by counts per degree sec (32) then take kalman
    // printf("%d\n",gyro.gx);

    // timer_delay_ms(500);
    // }
}
// --------

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

void welcomeMessage() {
    char* strWelcome = "Welcome to Kirby Drop!";
    int lengthWelcome = strlen(strWelcome);
    int x1 = _WIDTH / 2 - lengthWelcome * gl_get_char_width() / 2;
    int y1 = _HEIGHT / 4;
    gl_draw_string(x1, y1, strWelcome, _BALLCOLOR);
    drawKirby(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 8) * 3, standing_kirby_right_map);
    timer_delay(1);

    // loop until user presses Start
    for (int i = 0; i < 3; i++) {
        char* str = "Press 'A' to Start";
        int length = strlen(str);
        int x = _WIDTH / 2 - length * gl_get_char_width() / 2;
        int y = _HEIGHT / 2;
        
        gl_draw_string(x1, y1, strWelcome, _BALLCOLOR);
        drawKirby(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 8) * 3, standing_kirby_right_map);
        gl_draw_string(x, y, str, _BALLCOLOR);
        timer_delay(1);
        gl_swap_buffer();
        gl_clear(GL_WHITE);
        gl_draw_string(x1, y1, strWelcome, _BALLCOLOR);
        drawKirby(_WIDTH / 2 - _KIRBYWIDTH / 2, (_HEIGHT / 8) * 3, standing_kirby_right_map);
        gl_draw_string(x, y, str, GL_WHITE);
        timer_delay(1);
        gl_swap_buffer();
        gl_clear(GL_WHITE);
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
    for (int i = 0; i < _NPLATFORMS; i++) {
        p = generateRandomPlatform();
        p.y = y_coord;
        platforms[i] = p;
        y_coord += ( _HEIGHT / _NPLATFORMS);
    }
}

void gameOver() {
    timer_delay(3);
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
        readGyro();
        // moves ball up with platform
        if (gyro.gx - gyro.oldGx >= 2000) {
            if (gyro.ax - gyro.oldAx <= 3000) {
                //move right
                printf("Moving right \n");
                x_coord += 1;
            } else if (gyro.oldAx - gyro.ax <= 3000) {
                //move left
                printf("Moving left \n");
                x_coord -= 1;
            } else {
                printf("no movement\n");
            }
        }
        y_coord -= 1;
        drawKirby(x_coord, y_coord, standing_kirby_left_map);
        // if ball collides with platform
        if (drawKirby(x_coord, y_coord, standing_kirby_left_map) == 0) {
            // if not touching left edge of screen
            if (x_coord > 0) {
                // moves ball to the left
                x_coord -= 1;
            }
            y_coord -= 1;
            drawKirby(x_coord, y_coord, standing_kirby_left_map);
        } else {
            // ball continues free-falling
            y_coord++;
            drawKirby(x_coord, y_coord, standing_kirby_left_map);
        }
        // ball hits top or bottom edge of screen
        if (y_coord + _BALLRADIUS == _HEIGHT || y_coord - _BALLRADIUS == 0) {
            break;
        }
        gl_swap_buffer();
        timer_delay_ms(10);
    }
    gameOver();
}

// main program
void main(void) {
    init_window();
    welcomeMessage();
    generatePlatforms();
    scrollScreen();
    gl_swap_buffer();
}











// possible tests
// gl_draw_pixel(_WIDTH/3, _HEIGHT/3, GL_AMBER);
// assert(gl_read_pixel(_WIDTH/3, _HEIGHT/3) == GL_AMBER);
// gl_draw_rect(_WIDTH/2 - 20, _HEIGHT/2 - 20, 4, 50, GL_BLUE);
// gl_draw_char(60, 10, 'A', GL_BLUE);
// gl_draw_string(300, 300, "Hi Eric", GL_AMBER);
// gl_swap_buffer();
