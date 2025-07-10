#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <stdlib.h>

int read_reg(int file, int addr, int reg) {
    unsigned char buf[1] = { reg };
    if (write(file, buf, 1) != 1) return -1;
    if (read(file, buf, 1) != 1) return -1;
    return buf[0];
}

void print_header() {
    printf("╔═══════════════════════════════════════════════╦════════════╦════════════════════════════════════════════╗\n");
    printf("║ Register Description                         ║ Hex Value  ║ Interpretation                              ║\n");
    printf("╠═══════════════════════════════════════════════╬════════════╬════════════════════════════════════════════╣\n");
}

void print_row(const char *label, int value, const char *meaning) {
    printf("║ %-45s ║  0x%02X     ║ %-42s ║\n", label, value, meaning);
}

void print_footer() {
    printf("╚═══════════════════════════════════════════════╩════════════╩════════════════════════════════════════════╝\n");
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

    print_header();

    int reg00 = read_reg(file, addr, 0x00);
    print_row("PMU Status (0x00)", reg00, "-");

    int reg01 = read_reg(file, addr, 0x01);
    print_row("Power-On Reason (0x01)", reg01, "-");

    int acv = ((read_reg(file, addr, 0x56) << 4) | (read_reg(file, addr, 0x57) & 0x0F));
    int acc = ((read_reg(file, addr, 0x58) << 4) | (read_reg(file, addr, 0x59) & 0x0F));
    char acvolt[32], accurr[32];
    snprintf(acvolt, sizeof(acvolt), "%dmV", (int)(acv * 1.7));
    snprintf(accurr, sizeof(accurr), "%dmA", (int)(acc * 0.625));
    print_row("ACIN Voltage (0x56/0x57)", acv, acvolt);
    print_row("ACIN Current (0x58/0x59)", acc, accurr);

    int vbv = ((read_reg(file, addr, 0x5A) << 4) | (read_reg(file, addr, 0x5B) & 0x0F));
    int vbc = ((read_reg(file, addr, 0x5C) << 4) | (read_reg(file, addr, 0x5D) & 0x0F));
    char vbvolt[32], vbcurr[32];
    snprintf(vbvolt, sizeof(vbvolt), "%dmV", (int)(vbv * 1.7));
    snprintf(vbcurr, sizeof(vbcurr), "%dmA", (int)(vbc * 0.375));
    print_row("VBUS Voltage (0x5A/0x5B)", vbv, vbvolt);
    print_row("VBUS Current (0x5C/0x5D)", vbc, vbcurr);

    int reg33 = read_reg(file, addr, 0x33);
    int charge_idx = (reg33 >> 4) & 0x7;
    int charge_mA[] = { 100, 190, 280, 360, 450, 550, 630, 700 };
    char charge_str[64];
    snprintf(charge_str, sizeof(charge_str), "%s (%dmA)",
             (reg33 & 0x80) ? "Enabled" : "Disabled", charge_mA[charge_idx]);
    print_row("Charge Control 1 (0x33)", reg33, charge_str);

    int reg34 = read_reg(file, addr, 0x34);
    print_row("Charge Control 2 (0x34)", reg34, "-");

    int reg36 = read_reg(file, addr, 0x36);
    char status[64] = "";
    if (reg36 & 0x80) strcat(status, "Charging ");
    if (reg36 & 0x40) strcat(status, "Full ");
    if (reg36 & 0x20) strcat(status, "USB ");
    if (reg36 & 0x04) strcat(status, "Battery ");
    print_row("Charge Status (0x36)", reg36, status[0] ? status : "-");

    int reg3A = read_reg(file, addr, 0x3A);
    int reg3B = read_reg(file, addr, 0x3B);
    int minutes = (reg3A << 4) | (reg3B & 0x0F);
    char timer_str[32];
    snprintf(timer_str, sizeof(timer_str), "%d minutes", minutes);
    print_row("Charge Timer (0x3A/0x3B)", minutes, timer_str);

    int reg7A = read_reg(file, addr, 0x7A);
    int reg7B = read_reg(file, addr, 0x7B);
    int vbat_raw = ((reg7A << 4) | (reg7B & 0x0F));
    char vbat_info[32];
    snprintf(vbat_info, sizeof(vbat_info), "%dmV", (int)(vbat_raw * 1.1));
    print_row("Battery Voltage (0x7A/0x7B)", vbat_raw, vbat_info);

    int reg78 = read_reg(file, addr, 0x78);
    char fuel_str[32];
    snprintf(fuel_str, sizeof(fuel_str), "Fuel Gauge Ctrl: 0x%02X", reg78);
    print_row("Fuel Gauge Control (0x78)", reg78, fuel_str);

    int charge_current = ((read_reg(file, addr, 0x7E) << 5) | (read_reg(file, addr, 0x7F) & 0x1F));
    char chgcurr[32];
    snprintf(chgcurr, sizeof(chgcurr), "%dmA", charge_current);
    print_row("Charge Current (0x7E/0x7F)", charge_current, chgcurr);

    int regB9 = read_reg(file, addr, 0xB9);
    char soc_str[32];
    snprintf(soc_str, sizeof(soc_str), "%d%%", regB9 & 0x7F);
    print_row("Fuel Gauge (0xB9)", regB9, soc_str);

    for (int r = 0xBA; r <= 0xBE; r++) {
        int val = read_reg(file, addr, r);
        char desc[64];
        snprintf(desc, sizeof(desc), "Status Register (0x%02X)", r);
        print_row(desc, val, "-");
    }

    print_footer();
    return 0;
}
