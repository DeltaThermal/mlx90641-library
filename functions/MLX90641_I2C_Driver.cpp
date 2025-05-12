// functions/MLX90641_I2C_Driver.cpp
#include "MLX90641_I2C_Driver.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdio.h>

static int i2c_fd = -1;

// Open the I2C bus if not already open
static void ensure_i2c_open() {
    if (i2c_fd < 0) {
        const char *dev = "/dev/i2c-1";
        if ((i2c_fd = open(dev, O_RDWR)) < 0) {
            perror("Opening I2C device");
        }
    }
}

void MLX90641_I2CInit(void) {
    ensure_i2c_open();
}

int MLX90641_I2CGeneralReset(void) {
    // No-op on Linux
    return 0;
}

int MLX90641_I2CRead(uint8_t slaveAddr,
                     uint16_t startAddress,
                     uint16_t nMemAddressRead,
                     uint16_t *data) {
    ensure_i2c_open();
    // Read in blocks to avoid exceeding kernel I2C msg size limits
    const int maxWords = 32;
    uint16_t addr = startAddress;
    int remaining = nMemAddressRead;
    int index = 0;
    while (remaining > 0) {
        int blockWords = remaining > maxWords ? maxWords : remaining;
        int bytes = blockWords * 2;
        uint8_t buf[64]; // maxWords*2
        uint8_t reg[2] = { static_cast<uint8_t>(addr >> 8),
                           static_cast<uint8_t>(addr & 0xFF) };

        struct i2c_rdwr_ioctl_data packets;
        struct i2c_msg msgs[2];

        // Write register address
        msgs[0].addr  = slaveAddr;
        msgs[0].flags = 0;
        msgs[0].len   = 2;
        msgs[0].buf   = reg;

        // Read data into buf
        msgs[1].addr  = slaveAddr;
        msgs[1].flags = I2C_M_RD;
        msgs[1].len   = bytes;
        msgs[1].buf   = buf;

        packets.msgs  = msgs;
        packets.nmsgs = 2;

        if (ioctl(i2c_fd, I2C_RDWR, &packets) < 0) {
            perror("I2C_RDWR ioctl");
            return -1;
        }

        // Pack into uint16_t array (MSB first)
        for (int i = 0; i < blockWords; ++i) {
            data[index + i] = (uint16_t(buf[2*i]) << 8) | uint16_t(buf[2*i + 1]);
        }

        // advance pointers
        addr += blockWords;
        index += blockWords;
        remaining -= blockWords;
    }
    return 0;
}

void MLX90641_I2CFreqSet(int /*freq*/) {
    // No-op on Linux
}

int MLX90641_I2CWrite(uint8_t slaveAddr,
                      uint16_t writeAddress,
                      uint16_t value) {
    ensure_i2c_open();
    // Compose 4-byte buffer: addr MSB/LSB, value MSB/LSB
    uint8_t buf[4] = { static_cast<uint8_t>(writeAddress >> 8),
                       static_cast<uint8_t>(writeAddress & 0xFF),
                       static_cast<uint8_t>(value >> 8),
                       static_cast<uint8_t>(value & 0xFF) };
    if (ioctl(i2c_fd, I2C_SLAVE, slaveAddr) < 0) {
        perror("I2C: set slave");
        return -1;
    }
    if (write(i2c_fd, buf, 4) != 4) {
        perror("I2C: Write reg+data");
        return -1;
    }
    return 0;
}
