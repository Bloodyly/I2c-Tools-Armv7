#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>


#define MAX_REGISTERS 128

typedef struct {
    uint8_t address;
    const char* name;
    const char* section;
    uint8_t value;
} AXP223_Register;

int read_reg(int fd, uint8_t addr) {
    if (write(fd, &addr, 1) != 1) {
        perror("Write register address failed");
        return -1;
    }
    unsigned char buf;
    if (read(fd, &buf, 1) != 1) {
        perror("Read register failed");
        return -1;
    }
    return buf;
}

// ==========================
// Alle relevanten Register
// ==========================
AXP223_Register registers[] = {
    // Power Control (10.1.1)
    {0x00, "Power Status", "Power Control"},
    {0x01, "Charge State", "Power Control"},
    {0x10, "Power Enable 1", "Power Control"},
    {0x12, "Power Enable 2", "Power Control"},
    {0x13, "ALDO3 Enable", "Power Control"},
    {0x15, "DLDO1 Voltage", "Power Control"},
    {0x16, "DLDO2 Voltage", "Power Control"},
    {0x17, "DLDO3 Voltage", "Power Control"},
    {0x18, "DLDO4 Voltage", "Power Control"},
    {0x19, "ELDO1 Voltage", "Power Control"},
    {0x1A, "ELDO2 Voltage", "Power Control"},
    {0x1B, "ELDO3 Voltage", "Power Control"},
    {0x1C, "DC5LDO Voltage", "Power Control"},
    {0x21, "DCDC1 Voltage", "Power Control"},
    {0x22, "DCDC2 Voltage", "Power Control"},
    {0x23, "DCDC3 Voltage", "Power Control"},
    {0x24, "DCDC4 Voltage", "Power Control"},
    {0x25, "DCDC5 Voltage", "Power Control"},
    {0x27, "DCDC2/3 Ramp", "Power Control"},
    {0x28, "ALDO1 Voltage", "Power Control"},
    {0x29, "ALDO2 Voltage", "Power Control"},
    {0x2A, "ALDO3 Voltage", "Power Control"},
    {0x30, "VBUS Path Ctrl", "Power Control"},
    {0x31, "Wakeup + VOFF", "Power Control"},
    {0x32, "Shutdown + LED", "Power Control"},
    {0x33, "Charge Ctrl 1", "Power Control"},
    {0x34, "Charge Ctrl 2", "Power Control"},
    {0x35, "Charge Ctrl 3", "Power Control"},
    {0x36, "PEK Parameters", "Power Control"},
    {0x37, "DCDC Frequency", "Power Control"},
    {0x38, "Temp Warn Low", "Power Control"},
    {0x39, "Temp Warn High", "Power Control"},
    {0x3C, "Temp Warn Disch Low", "Power Control"},
    {0x3D, "Temp Warn Disch High", "Power Control"},

    // GPIO Control (10.1.2)
    {0x90, "GPIO0 Ctrl", "GPIO"},
    {0x91, "GPIO0 Voltage", "GPIO"},
    {0x92, "GPIO1 Ctrl", "GPIO"},
    {0x93, "GPIO1 Voltage", "GPIO"},
    {0x94, "GPIO Status", "GPIO"},
    {0x97, "GPIO Pull-Down", "GPIO"},

    // Interrupt Control (10.1.3)
    {0x40, "IRQ Enable 1", "Interrupt"},
    {0x41, "IRQ Enable 2", "Interrupt"},
    {0x42, "IRQ Enable 3", "Interrupt"},
    {0x43, "IRQ Enable 4", "Interrupt"},
    {0x44, "IRQ Enable 5", "Interrupt"},
    {0x48, "IRQ Status 1", "Interrupt"},
    {0x49, "IRQ Status 2", "Interrupt"},
    {0x4A, "IRQ Status 3", "Interrupt"},
    {0x4B, "IRQ Status 4", "Interrupt"},
    {0x4C, "IRQ Status 5", "Interrupt"},

    // ADC Data (10.1.4)
    {0x56, "Temp ADC High", "ADC"},
    {0x57, "Temp ADC Low", "ADC"},
    {0x58, "TS ADC High", "ADC"},
    {0x59, "TS ADC Low", "ADC"},
    {0x78, "Battery Volt High", "ADC"},
    {0x79, "Battery Volt Low", "ADC"},
    {0x7A, "Charge Curr High", "ADC"},
    {0x7B, "Charge Curr Low", "ADC"},
    {0x7C, "Discharge Curr High", "ADC"},
    {0x7D, "Discharge Curr Low", "ADC"},
	
	    // Weitere Steuer-/Fuel-Gauge/ADC-Register
    {0x80, "DCDC Mode", "ADC"},
    {0x82, "ADC Enable", "ADC"},
    {0x84, "ADC Sample + TS", "ADC"},
    {0x85, "TS Sample Rate", "ADC"},
    {0x8A, "Timer Control", "Timer"},
    {0x8C, "PWREN Control 1", "Power Control"},
    {0x8D, "PWREN Control 2", "Power Control"},
    {0x8F, "Overtemp Shutdown", "Power Control"},
    {0xB8, "Fuel Gauge Ctrl", "Fuel Gauge"},
    {0xB9, "Fuel Gauge Result", "Fuel Gauge"},
    {0xE0, "Battery Cap Hi", "Fuel Gauge"},
    {0xE1, "Battery Cap Lo", "Fuel Gauge"},
    {0xE6, "Battery Alarm", "Fuel Gauge"},
    {0xE8, "FG Update Interval", "Fuel Gauge"},
    {0xE9, "FG Calibration Interval", "Fuel Gauge"},
    {0xEC, "FG Capacity % Calib", "Fuel Gauge"}

};

// ==========================
// Tabellen-Ausgabe
// ==========================
void print_table(AXP223_Register* regs, size_t count) {
    printf("\n=== AXP223 Register Dump ===\n");
    printf("| Addr | Section     | Name                  | Value (Hex) |\n");
    printf("|------|-------------|------------------------|-------------|\n");

    for (size_t i = 0; i < count; ++i) {
        printf("| 0x%02X | %-11s | %-22s |     0x%02X     |\n",
               regs[i].address, regs[i].section, regs[i].name, regs[i].value);
    }
    printf("\n");
}

// ==========================
// Interpretations
// ==========================

void interpret_reg00(uint8_t value) {
    printf("REG00 (Power Input Status): 0x%02X\n", value);
    printf("  - ACIN Present         : %s\n", (value & 0x80) ? "Yes" : "No");
    printf("  - ACIN Usable          : %s\n", (value & 0x40) ? "Yes" : "No");
    printf("  - VBUS Present         : %s\n", (value & 0x20) ? "Yes" : "No");
    printf("  - VBUS Usable          : %s\n", (value & 0x10) ? "Yes" : "No");
    printf("  - VBUS > VHOLD         : %s\n", (value & 0x08) ? "Yes" : "No");
    printf("  - Battery direction        : %s\n", (value & 0x04) ? "charging" : "discharging");
    printf("  - ACIN/VBUS Short      : %s\n", (value & 0x02) ? "Yes" : "No");
    printf("  - Power-on via ExtPwr  : %s\n", (value & 0x01) ? "Yes" : "No");
}

void interpret_reg01(uint8_t value) {
    printf("REG01 (Power Mode / Charge State): 0x%02X\n", value);
    printf("  - Overtemperature       : %s\n", (value & 0x80) ? "Yes" : "No");
    printf("  - Charging              : %s\n", (value & 0x40) ? "Yes" : "No");
    printf("  - Battery Present       : %s\n", (value & 0x20) ? "Yes" : "No");
    printf("  - Battery Activation    : %s\n", (value & 0x08) ? "Yes" : "No");
}

void interpret_reg32(uint8_t value) {
    printf("REG32 (Shutdown, Battery Detection, CHGLED Control): 0x%02X\n", value);
    printf("  - Software Shutdown (bit 7)          : %s\n", (value & 0x80) ? "Active (shutdown requested)" : "Inactive");
    printf("  - Battery Detection (bit 6)          : %s\n", (value & 0x40) ? "Enabled" : "Disabled");
    // CHGLED Control Mode (Bit 5–4)
    uint8_t chg_mode = (value >> 4) & 0x03;
    const char* chgled_modes[] = {
        "High-Z (off)",
        "25%% Duty @ 0.5 Hz (slow blink)",
        "25%% Duty @ 2 Hz (fast blink)",
        "Low Level (ON)"
    };
    printf("  - CHGLED Mode (bits 5–4)             : %s\n", chgled_modes[chg_mode]);
    // CHGLED Source Select (Bit 3)
    printf("  - CHGLED Source (bit 3)              : %s\n", (value & 0x08) ? "Automatic (charger-controlled)" : "Manual (via bits 5–4)");
    // Shutdown Sequence Timing (Bit 2)
    printf("  - Power-Off Timing (bit 2)           : %s\n", (value & 0x04) ? "Sequenced off (reverse start order)" : "All at once");
    // PWROK Delay (Bits 1–0)
    uint8_t delay = value & 0x03;
    const char* delays[] = { "8 ms", "16 ms", "32 ms", "64 ms" };
    printf("  - PWROK Delay (bits 1–0)             : %s after last rail\n", delays[delay]);
}

void interpret_reg33(uint8_t value) {
    printf("REG33 (Charging Control 1): 0x%02X\n", value);
    // Bit 7: Enable Charging
    printf("  - Charging Enabled (bit 7)       : %s\n", (value & 0x80) ? "Yes" : "No");
    // Bits 6–5: Target Voltage (VTRGT)
    const char* vtrgt_levels[] = {"4.1V", "4.22V", "4.2V", "4.24V"};
    uint8_t vtrgt = (value >> 5) & 0x03;
    printf("  - Charge Target Voltage (6:5)   : %s\n", vtrgt_levels[vtrgt]);
    // Bit 4: Charge Termination Current Threshold
    printf("  - End-of-Charge Threshold       : %s\n", (value & 0x10) ? "<15%% of CC" : "<10%% of CC");
    // Bits 3–0: Charge Current
    uint8_t cc_bits = value & 0x0F;
    int cc_values[] = {
        300, 450, 600, 750,
        900, 1050, 1200, 1350,
        1500, 1650, 1800, 1950,
        2100, -1, -1, -1 // ungültige Werte
    };
    int current = cc_values[cc_bits];
    if (current > 0)
        printf("  - Charge Current (3:0)          : %d mA\n", current);
    else
        printf("  - Charge Current (3:0)          : Invalid setting (reserved)\n");
}

void interpret_battery_voltage(uint8_t reg78, uint8_t reg79) {
    int raw = (reg78 << 4) | (reg79 & 0x0F);
    float voltage_mV = raw * 1.1f;
    printf("Battery Voltage         : %.1f mV (RAW: 0x%03X)\n", voltage_mV, raw);
}

void interpret_charge_current(uint8_t reg7a, uint8_t reg7b) {
    int raw = (reg7a << 5) | (reg7b & 0x1F);
    float current_mA = raw * 0.5f;
    printf("Battery Charge Current  : %.1f mA (RAW: 0x%04X)\n", current_mA, raw);
}

void interpret_discharge_current(uint8_t reg7c, uint8_t reg7d) {
    int raw = (reg7c << 5) | (reg7d & 0x1F);
    float current_mA = raw * 0.5f;
    printf("Battery Discharge Current: %.1f mA (RAW: 0x%04X)\n", current_mA, raw);
}



void interpret_reg34(uint8_t value) {
    printf("REG34 (Charging Control 2): 0x%02X\n", value);
    // Bits 7–6: Pre-charge Timeout
    const char* precharge_timeout[] = {"40 min", "50 min", "60 min", "70 min"};
    printf("  - Pre-charge Timeout (7:6)     : %s\n", precharge_timeout[(value >> 6) & 0x03]);
    // Bit 5: Keep charge output after full
    printf("  - Keep Charging Output (bit 5) : %s\n", (value & 0x20) ? "Yes (remain active)" : "No (disable)");
    // Bit 4: CHGLED Type
    printf("  - CHGLED Mode Type (bit 4)     : %s\n", (value & 0x10) ? "Type B (blink)" : "Type A (solid/low)");
    // Bit 2: Constant Current sync with setting
    printf("  - Sync on Current Change (bit 2): %s\n", (value & 0x04) ? "Yes (dynamic adjust)" : "No (ignore until reset)");
    // Bits 1–0: CC Timeout
    const char* cc_timeout[] = {"6h", "8h", "10h", "12h"};
    printf("  - CC Timeout (1:0)             : %s\n", cc_timeout[value & 0x03]);
}

void interpret_reg35(uint8_t value) {
    printf("REG35 (Charging Control 3): 0x%02X\n", value);

    // Nur Bits 3–0 sind gültig
    uint8_t cc_bits = value & 0x0F;
    int charge_current[] = {
        300, 450, 600, 750,
        900, 1050, 1200, 1350,
        1500, 1650, 1800, 1950,
        2100, 2250, -1, -1
    };
    int current = charge_current[cc_bits];

    if (current > 0)
        printf("  - Loop Charging Current Limit : %d mA\n", current);
    else
        printf("  - Loop Charging Current Limit : Invalid/Reserved\n");
}

void interpret_adc_voltage(const char* label, uint8_t high, uint8_t low, float scale_mV) {
    int raw = (high << 4) | (low & 0x0F);
    float voltage = raw * scale_mV;
    printf("%s Voltage           : %.2f mV (RAW: 0x%03X)\n", label, voltage, raw);
}

void interpret_adc_current(const char* label, uint8_t high, uint8_t low, float scale_mA) {
    int raw = (high << 5) | (low & 0x1F);
    float current = raw * scale_mA;
    printf("%s Current           : %.2f mA (RAW: 0x%04X)\n", label, current, raw);
}

void interpret_internal_temp(uint8_t high, uint8_t low) {
    int raw = (high << 4) | (low & 0x0F);
    float temp = raw * 0.1f - 144.7f;
    printf("Internal Temperature   : %.1f °C (RAW: 0x%03X)\n", temp, raw);
}

void interpret_reg82(uint8_t value) {
    printf("REG82 (ADC Enable 1): 0x%02X\n", value);

    printf("  - TS Pin ADC Enable           : %s\n", (value & 0x80) ? "Yes" : "No");
    printf("  - Battery Voltage ADC Enable  : %s\n", (value & 0x40) ? "Yes" : "No");
    printf("  - Battery Current ADC Enable  : %s\n", (value & 0x20) ? "Yes" : "No");

    // Bits 4–0 sind nicht dokumentiert in AXP223 v1.1
    printf("  - Other ADCs (undocumented)   : 0x%02X (raw bits 4–0)\n", value & 0x1F);
}
void interpret_reg_b8(uint8_t value) {
    printf("REG B8 (Fuel Gauge Control): 0x%02X\n", value);

    printf("  - Fuel Gauge Enabled (bit 7)              : %s\n", (value & 0x80) ? "Yes" : "No");
    printf("  - Coulomb Counter Enabled (bit 6)         : %s\n", (value & 0x40) ? "Yes" : "No");
    printf("  - Capacity Calibration Trigger (bit 5)    : %s\n", (value & 0x20) ? "Requested" : "Not triggered");
    printf("  - Calibration Status (bit 4)              : %s\n", (value & 0x10) ? "Not running" : "Running");
    printf("  - Reserved Bits (3:0)                     : 0x%X\n", value & 0x0F);
}

void interpret_reg_e0_e1(uint8_t reg_e0, uint8_t reg_e1) {
    printf("REG E0 (Battery Capacity 1): 0x%02X\n", reg_e0);
    printf("REG E1 (Battery Capacity 2): 0x%02X\n", reg_e1);

    bool capacity_valid = reg_e0 & 0x80;
    uint16_t raw = ((reg_e0 & 0x7F) << 8) | reg_e1;
    float capacity_mAh = raw * 1.456f;

    printf("  - Capacity Configured (bit 7)   : %s\n", capacity_valid ? "Yes" : "No");
    printf("  - Raw Capacity Setting (14:0)   : %u (0x%04X)\n", raw, raw);
    printf("  - Calculated Battery Capacity   : %.2f mAh\n", capacity_mAh);
}

void interpret_reg48(uint8_t value) {
    printf("REG 48h (IRQ Status 1): 0x%02X\n", value);
    if (value & 0x80) puts("  - ACIN Over-Voltage");
    if (value & 0x40) puts("  - ACIN Connected");
    if (value & 0x20) puts("  - ACIN Removed");
    if (value & 0x10) puts("  - VBUS Over-Voltage");
    if (value & 0x08) puts("  - VBUS Connected");
    if (value & 0x04) puts("  - VBUS Removed");
    if (value & 0x02) puts("  - VBUS Below VHOLD");
}

void interpret_reg49(uint8_t value) {
    printf("REG 49h (IRQ Status 2): 0x%02X\n", value);
    if (value & 0x80) puts("  - Battery Connected");
    if (value & 0x40) puts("  - Battery Removed");
    if (value & 0x20) puts("  - Battery Entered Activate Mode");
    if (value & 0x10) puts("  - Battery Exited Activate Mode");
    if (value & 0x08) puts("  - Battery is Charging");
    if (value & 0x04) puts("  - Charging Finished");
    if (value & 0x02) puts("  - Battery Over-Temperature");
    if (value & 0x01) puts("  - Battery Low-Temperature");
}

void interpret_reg4a(uint8_t value) {
    printf("REG 4Ah (IRQ Status 3): 0x%02X\n", value);
    if (value & 0x80) puts("  - Internal Over-Temperature");
    if (value & 0x02) puts("  - PEK Short Press");
    if (value & 0x01) puts("  - PEK Long Press");
}

void interpret_reg4b(uint8_t value) {
    printf("REG 4Bh (IRQ Status 4): 0x%02X\n", value);
    if (value & 0x02) puts("  - Battery Alarm Threshold 1 Reached");
    if (value & 0x01) puts("  - Battery Alarm Threshold 2 Reached");
}

void interpret_reg4c(uint8_t value) {
    printf("REG 4Ch (IRQ Status 5): 0x%02X\n", value);
    if (value & 0x80) puts("  - Timer Timeout");
    if (value & 0x40) puts("  - PEK Rising Edge");
    if (value & 0x20) puts("  - PEK Falling Edge");
    if (value & 0x02) puts("  - GPIO1 Edge Trigger");
    if (value & 0x01) puts("  - GPIO0 Edge Trigger");
}

void interpret_all_irq_status(int fd) {
    uint8_t r48 = read_reg(fd, 0x48);
    uint8_t r49 = read_reg(fd, 0x49);
    uint8_t r4a = read_reg(fd, 0x4A);
    uint8_t r4b = read_reg(fd, 0x4B);
    uint8_t r4c = read_reg(fd, 0x4C);

    printf("=== AXP223 IRQ Status Overview ===\n");
    interpret_reg48(r48);
    interpret_reg49(r49);
    interpret_reg4a(r4a);
    interpret_reg4b(r4b);
    interpret_reg4c(r4c);
}


void read_and_interpret_registers(int fd) {
    // === Einzelregister lesen ===
    uint8_t reg00 = read_reg(fd, 0x00);
    uint8_t reg01 = read_reg(fd, 0x01);
    uint8_t reg32 = read_reg(fd, 0x32);
    uint8_t reg33 = read_reg(fd, 0x33);
    uint8_t reg34 = read_reg(fd, 0x34);
    uint8_t reg35 = read_reg(fd, 0x35);
    uint8_t reg82 = read_reg(fd, 0x82);
    uint8_t regB8 = read_reg(fd, 0xB8);
    uint8_t regE0 = read_reg(fd, 0xE0);
    uint8_t regE1 = read_reg(fd, 0xE1);

    // === Paare lesen (für ADCs etc.) ===
    uint8_t reg78 = read_reg(fd, 0x78);
    uint8_t reg79 = read_reg(fd, 0x79);
    uint8_t reg7A = read_reg(fd, 0x7A);
    uint8_t reg7B = read_reg(fd, 0x7B);
    uint8_t reg7C = read_reg(fd, 0x7C);
    uint8_t reg7D = read_reg(fd, 0x7D);
    uint8_t reg56 = read_reg(fd, 0x56);
    uint8_t reg57 = read_reg(fd, 0x57);

    // === IRQ Statusregister ===
    uint8_t reg48 = read_reg(fd, 0x48);
    uint8_t reg49 = read_reg(fd, 0x49);
    uint8_t reg4A = read_reg(fd, 0x4A);
    uint8_t reg4B = read_reg(fd, 0x4B);
    uint8_t reg4C = read_reg(fd, 0x4C);

    // === Interpretation in klarer Reihenfolge ===
    printf("\n=== AXP223 Register Interpretation ===\n");
    interpret_reg00(reg00);
    interpret_reg01(reg01);
    interpret_reg32(reg32);
    interpret_reg33(reg33);
    interpret_reg34(reg34);
    interpret_reg35(reg35);
    interpret_battery_voltage(reg78, reg79);
    interpret_charge_current(reg7A, reg7B);
    interpret_discharge_current(reg7C, reg7D);
    interpret_internal_temp(reg56, reg57);
    interpret_reg82(reg82);
    interpret_reg_b8(regB8);
    interpret_reg_e0_e1(regE0, regE1);
    interpret_reg48(reg48);
    interpret_reg49(reg49);
    interpret_reg4a(reg4A);
    interpret_reg4b(reg4B);
    interpret_reg4c(reg4C);
}


// ==========================
// Main
// ==========================
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <i2c-bus> <device-address-hex>\n", argv[0]);
        return 1;
    }

    const char* i2c_bus = argv[1];
    int addr = (int)strtol(argv[2], NULL, 16);

    int fd = open(i2c_bus, O_RDWR);
    if (fd < 0) {
        perror("Open I2C bus failed");
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        perror("Set I2C address failed");
        close(fd);
        return 1;
    }

    size_t reg_count = sizeof(registers) / sizeof(registers[0]);
    for (size_t i = 0; i < reg_count; ++i) {
        int val = read_reg(fd, registers[i].address);
        if (val < 0) {
            fprintf(stderr, "Failed to read register 0x%02X\n", registers[i].address);
            continue;
        }
        registers[i].value = val;
    }

    print_table(registers, reg_count);
	read_and_interpret_registers(fd);
    close(fd);
    return 0;
}
