#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int read_reg(int file, int addr, int reg) {
    unsigned char buf[1] = { reg };
    if (write(file, buf, 1) != 1) return -1;
    if (read(file, buf, 1) != 1) return -1;
    return buf[0];
}

int read_reg16(int file, int addr, int reg) {
    unsigned char buf[2];
    buf[0] = reg;
    if (write(file, buf, 1) != 1) return -1;
    if (read(file, buf, 2) != 2) return -1;
    return (buf[0] << 4) | (buf[1] & 0x0F);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <i2c-bus-path> <device-hex>\n", argv[0]);
        return 1;
    }

    const char* dev_path = argv[1];
    int addr = (int)strtol(argv[2], NULL, 16);
    int file = open(dev_path, O_RDWR);
    if (file < 0 || ioctl(file, I2C_SLAVE, addr) < 0) return 1;

    printf("=== AXP223 Register Dump & Interpretation ===\n");

    int reg33 = read_reg(file, addr, 0x33);
    int reg36 = read_reg(file, addr, 0x36);
    int reg78 = read_reg(file, addr, 0x78);
    int vbat  = read_reg16(file, addr, 0x7A);
    int idis  = read_reg16(file, addr, 0x7C);
    int ichg  = read_reg16(file, addr, 0x7E);
    int reg00 = read_reg(file, addr, 0x00);
    int reg01 = read_reg(file, addr, 0x01);
    const int currents[] = { 100, 190, 280, 360, 450, 550, 630, 700 };

    printf("Charge Control 1 (0x33): 0x%02x\n", reg33);
    printf("  Charging: %s\n", (reg33 & 0x80) ? "Enabled" : "Disabled");
    printf("  Charge current limit: %dmA\n", currents[(reg33 >> 4) & 0x7]);

    printf("Charge Status (0x36): 0x%02x\n", reg36);
    if (reg36 & 0x80) puts("  -> Charging active");
    if (reg36 & 0x40) puts("  -> Battery full");
    if (reg36 & 0x20) puts("  -> USB input detected");
    if (reg36 & 0x04) puts("  -> Battery connected");

    printf("Fuel Gauge (0x78): 0x%02x\n", reg78);
    printf("Battery Voltage (0x7A-0x7B): %dmV\n", vbat * 1.1);
    printf("Discharge Current (0x7C-0x7D): %dmA\n", idis);
    printf("Charge Current (0x7E-0x7F): %dmA\n", ichg);
    printf("PMU Status (0x00): 0x%02x\n", reg00);
    printf("Power-On Reason (0x01): 0x%02x\n", reg01);
    return 0;
}
