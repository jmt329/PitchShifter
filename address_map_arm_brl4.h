/* This files provides address values that exist in the system */

#define BOARD                 "DE1-SoC"

/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
//
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_END       0xC803FFFF
// modified for 640x480
// #define FPGA_ONCHIP_SPAN      0x00040000
#define FPGA_ONCHIP_SPAN      0x00080000
//
#define FPGA_CHAR_BASE        0xC9000000
#define FPGA_CHAR_END         0xC9001FFF
#define FPGA_CHAR_SPAN        0x00002000

/* Cyclone V FPGA devices */
#define HW_REGS_BASE          0xff200000
//#define HW_REGS_SPAN        0x00200000
#define HW_REGS_SPAN          0x00006000
// === now offsets from the BASE ===
#define LEDR_BASE             0x00000000
#define HEX3_HEX0_BASE        0x00000020
#define HEX5_HEX4_BASE        0x00000030
#define SW_BASE               0x00000040
#define KEY_BASE              0x00000050
#define JP1_BASE              0x00000060
#define JP2_BASE              0x00000070
#define PS2_BASE              0x00000100
#define PS2_DUAL_BASE         0x00000108
#define JTAG_UART_BASE        0x00001000
#define JTAG_UART_2_BASE      0x00001008
#define IrDA_BASE             0x00001020
#define TIMER_BASE            0x00002000
#define TIMER_2_BASE          0x00002020
#define AV_CONFIG_BASE        0x00003000
#define PIXEL_BUF_CTRL_BASE   0x00003020
#define CHAR_BUF_CTRL_BASE    0x00003030
#define AUDIO_BASE            0x00003040
#define VIDEO_IN_BASE         0x00003060
#define ADC_BASE              0x00004000
// INSERT ADDRESSES HERE
#define RESET_BASE            0x00005000
#define DELTA_LEFT_BASE       0x00005010
#define DELTA_RIGHT_BASE      0x00005020
#define TRIANGLE_RIGHT_BASE   0x00005030
#define TRIANGLE_LEFT_BASE    0x00005040
#define DELTA_MODE_RIGHT_BASE 0x00005050
#define DELTA_MODE_LEFT_BASE  0x00005060
/*
#define MAN_DONE_BASE         0x00005000
#define RESET_BASE            0x00005010
#define START_IMG_BASE        0x00005020
#define STEP_IMG_BASE         0x00005030
#define START_REAL_BASE       0x00005040
#define STEP_REAL_BASE        0x000050e0
*/

/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE        0xFF709000
#define I2C0_BASE             0xFFC04000
#define I2C1_BASE             0xFFC05000
#define I2C2_BASE             0xFFC06000
#define I2C3_BASE             0xFFC07000
#define HPS_TIMER0_BASE       0xFFC08000
#define HPS_TIMER1_BASE       0xFFC09000
#define HPS_TIMER2_BASE       0xFFD00000
#define HPS_TIMER3_BASE       0xFFD01000
#define FPGA_BRIDGE           0xFFD0501C

/* ARM A9 MPCORE devices */
#define   PERIPH_BASE         0xFFFEC000    // base address of peripheral devices
#define   MPCORE_PRIV_TIMER   0xFFFEC600    // PERIPH_BASE + 0x0600

/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg
/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST       0xFFFED000    // PERIPH_BASE + 0x1000
#define ICDDCR                0x00          // offset to distributor control reg
#define ICDISER               0x100         // offset to interrupt set-enable regs
#define ICDICER               0x180         // offset to interrupt clear-enable regs
#define ICDIPTR               0x800         // offset to interrupt processor targets regs
#define ICDICFR               0xC00         // offset to interrupt configuration regs
