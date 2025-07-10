
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

int read_reg(int fd, int addr) {
    if (write(fd, &addr, 1) != 1) {
        perror("Failed to write register address");
        return -1;
    }

    unsigned char buf;
    if (read(fd, &buf, 1) != 1) {
        perror("Failed to read register");
        return -1;
    }

    return buf;
}

void print_voltage(int reg_high, int reg_low) {
    int raw = (reg_high << 4) | (reg_low & 0x0F);
    float voltage = raw * 1.1;
    printf("Battery Voltage: %.1f mV\n", voltage);
}

void print_current(const char* label, int reg_high, int reg_low) {
    int raw = (reg_high << 5) | (reg_low & 0x1F);
    float current = raw * 0.5;
    printf("%s: %.1f mA\n", label, current);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <i2c-bus-path> <device-hex>\n", argv[0]);
        return 1;
    }

    const char *i2c_bus = argv[1];
    int addr = (int)strtol(argv[2], NULL, 16);

    int fd = open(i2c_bus, O_RDWR);
    if (fd < 0) {
        perror("Failed to open I2C bus");
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        perror("Failed to set I2C address");
        close(fd);
        return 1;
    }

    int reg78 = read_reg(fd, 0x78);
    int reg79 = read_reg(fd, 0x79);
    int reg7a = read_reg(fd, 0x7a);
    int reg7b = read_reg(fd, 0x7b);
    int reg7c = read_reg(fd, 0x7c);
    int reg7d = read_reg(fd, 0x7d);

    printf("=== AXP223 Battery Monitor ===\n");
    print_voltage(reg78, reg79);
    print_current("Charge Current", reg7a, reg7b);
    print_current("Discharge Current", reg7c, reg7d);

    close(fd);
    return 0;
}
