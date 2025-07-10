#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <i2c-bus-path> <device-hex> <register-hex>\n", argv[0]);
        return 1;
    }

    const char* dev_path = argv[1];
    int addr = (int)strtol(argv[2], NULL, 16);
    int reg = (int)strtol(argv[3], NULL, 16);

    int file = open(dev_path, O_RDWR);
    if (file < 0) {
        perror("open");
        return 1;
    }

    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        perror("ioctl");
        return 1;
    }

    unsigned char buf[1] = { reg };
    if (write(file, buf, 1) != 1) {
        perror("write");
        return 1;
    }

    if (read(file, buf, 1) != 1) {
        perror("read");
        return 1;
    }

    printf("Value at 0x%02x: 0x%02x\n", reg, buf[0]);
    return 0;
}
