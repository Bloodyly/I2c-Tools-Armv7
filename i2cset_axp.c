#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <i2c-bus> <i2c-addr> <register> <value>\n", argv[0]);
        return 1;
    }

    const char* dev = argv[1];
    int addr = (int)strtol(argv[2], NULL, 16);
    int reg = (int)strtol(argv[3], NULL, 16);
    int val = (int)strtol(argv[4], NULL, 16);

    int file = open(dev, O_RDWR);
    if (file < 0) {
        perror("Open i2c bus failed");
        return 1;
    }

    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        perror("Select i2c device failed");
        close(file);
        return 1;
    }

    unsigned char buf[2] = { reg, val };
    if (write(file, buf, 2) != 2) {
        perror("Write failed");
        close(file);
        return 1;
    }

    printf("Wrote 0x%02X to register 0x%02X on device 0x%02X\n", val, reg, addr);
    close(file);
    return 0;
}
