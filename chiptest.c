#include "printf.h"
#include "strings.h"
#include "i2c.h"
#include "timer.h"

#define    MPU9250_ADDRESS            0x68
#define    MAG_ADDRESS                0x0C

#define    GYRO_FULL_SCALE_250_DPS    0x00
#define    GYRO_FULL_SCALE_500_DPS    0x08
#define    GYRO_FULL_SCALE_1000_DPS   0x10
#define    GYRO_FULL_SCALE_2000_DPS   0x18

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

void main() {
    i2c_init();
    char buf[1024];
    while (1) {
        char acc_register = 0x3B;
        i2c_write(MPU9250_ADDRESS,&acc_register,1);
        // read accelerometer
        i2c_read(MPU9250_ADDRESS,buf,14); 

        // convert to xyz values 
        int ax=-(buf[0]<<8 | buf[1]);
        int ay=-(buf[2]<<8 | buf[3]);
        int az=buf[4]<<8 | buf[5];
        int gx = buf[8]<<8 | buf[9]; // divide by counts per degree sec (32) then take kalman
        int gy = buf[10]<<8 | buf[11];
        printf("%d,%d,%d\n",ax,ay,az);
        timer_delay_ms(1000);

    }
}
