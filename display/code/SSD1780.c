#ifndef _OLEDV2_
#define _OLEDV2_

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

// #include "tco_libd.h"

/* Display    : https://www.vishay.com/docs/37902/oled128o064dbpp3n00000.pdf */
/* Data sheet : https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf */
/* Guide      : https://elinux.org/Interfacing_with_I2C_Devices */

#define I2C_INTERFACE "/dev/i2c-1" /* I2C2 (pins 3 & 4 on Coral) */
#define SLAVE_ADDR_WR 0b01111001 /* slave address write */
#define SLAVE_ADDR_RO 0b01111000 /* slave address read */

#define SSD1780_COMMAND_CTRL 0b01111000 /* D/C# bit set to command control */
#define SSD1780_COMMAND_DISP 0b01111010 /* D/C# bit set to display control */


/* This holds state of the OLED Display. */
typedef struct SSD1780_handle_t
{
    int fd;
} SSD1780_handle_t;

int ssd1780_init(SSD1780_handle_t *const handle) {
    if ((handle->fd = open(I2C_INTERFACE, O_RDWR)) < 0) { /* Open the bus */
        fprintf(stderr, "Could not open I2C (\'" I2C_INTERFACE "\') bus \n");
        return errno;
    }

    if (ioctl(handle->fd, I2C_SLAVE, SLAVE_ADDR_WR) < 0) {
        fprintf(stderr, "Failed to acquire bus access and/or talk to slave \n");
        return errno;
    }

}

int ssd1780_write(SSD1780_handle_t *const handle, uint8_t *const buf, size_t size) {
    /* TODO: configuration register to write to?? */
    if (write(handle->fd, buf, size) < 0) {
        fprintf(stderr, "Failed to write to the i2c bus : %s \n", g_strerror(errno));
        return errno;
    }

    return 0;
}

int ssd1780_read(SSD1780_handle_t *const handle, uint8_t *const buf, size_t size) {
    return -ENOSYS;
}

void ssd1780_shutdown(SSD1780_handle_t *const handle) {
    close(handle->fd);
}

#endif /* _OLEDV2_ */