#include <i2c.h>

int main(void) {

	// void i2c_init(void);
	// void i2c_read(unsigned slave_address, char *data, int data_length);
	// void i2c_write(unsigned slave_address, char *data, int data_length);

	i2c_init();
    int fd = 0;
    #define CTRL_REG1 0x20

    /* Power down the device (clean start) */
    i2c_write(fd, CTRL_REG1, 0x00)
    // i2c_smbus_write_byte_data(fd, CTRL_REG1, 0x00);

    /* Turn on the humidity sensor analog front end in single shot mode  */
    i2c_write(fd, CTRL_REG1, 0x84)
    // i2c_smbus_write_byte_data(fd, CTRL_REG1, 0x84);

    /* Run one-shot measurement (temperature and humidity). The set bit will be reset by the
     * sensor itself after execution (self-clearing bit) */
    i2c_write(fd, CTRL_REG2, 0x01)
    // i2c_smbus_write_byte_data(fd, CTRL_REG2, 0x01);

    /* Wait until the measurement is completed */
    do {
		delay(25);		/* 25 milliseconds */
		status = i2c_read(fd, CTRL_REG2)
		// status = i2c_smbus_read_byte_data(fd, CTRL_REG2);
    } while (status != 0);
}