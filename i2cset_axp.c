#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int read_reg(int file, int reg) {
    unsigned char buf[1] = { reg };
    if (write(file, buf, 1) != 1) return -1;
    if (read(file, buf, 1) != 1) return -1;
    return buf[0];
}

int write_reg(int file, int reg, int val) {
    unsigned char buf[2] = { reg, val };
    if (write(file, buf, 2) != 2) return -1;
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <i2c-bus-path> <current-mA>\n", argv[0]);
        return 1;
    }

    const char* dev = argv[1];
    int current = atoi(argv[2]);
    int file = open(dev, O_RDWR);
    if (file < 0 || ioctl(file, I2C_SLAVE, 0x34) < 0) return 1;

    int reg33 = read_reg(file, 0x33);
    if (reg33 < 0) return 1;

    int idx = -1;
    const int values[] = { 100, 190, 280, 360, 450, 550, 630, 700 };
    for (int i = 0; i < 8; i++) if (values[i] == current) idx = i;
    if (idx == -1) {
        fprintf(stderr, "Unsupported current. Use: 100–700 mA in defined steps.\n");
        return 1;
    }

    reg33 &= ~0x70;
    reg33 |= (idx << 4);
    reg33 |= 0x80; // ensure charging is enabled

    if (write_reg(file, 0x33, reg33) < 0) {
        perror("write");
        return 1;
    }

    printf("Set charge current to %dmA successfully.\n", current);
    return 0;
}
