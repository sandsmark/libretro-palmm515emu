#pragma once

#include <stdint.h>

#include <boolean.h>

//interrupt names
#define INT_EMIQ   0x00800000 //level 7
#define INT_RTI    0x00400000 //level 4
#define INT_SPI1   0x00200000 //level 1-6, configurable, datasheet is contraditory on this one
#define INT_IRQ5   0x00100000 //level 5
#define INT_IRQ6   0x00080000 //level 6
#define INT_IRQ3   0x00040000 //level 3
#define INT_IRQ2   0x00020000 //level 2
#define INT_IRQ1   0x00010000 //level 1
#define INT_PWM2   0x00002000 //level 1-6, configurable
#define INT_UART2  0x00001000 //level 1-6, configurable
#define INT_INT3   0x00000800 //level 4
#define INT_INT2   0x00000400 //level 4
#define INT_INT1   0x00000200 //level 4
#define INT_INT0   0x00000100 //level 4
#define INT_PWM1   0x00000080 //level 6
#define INT_KB     0x00000040 //level 4
#define INT_TMR2   0x00000020 //level 1-6, configurable
#define INT_RTC    0x00000010 //level 4
#define INT_WDT    0x00000008 //level 4
#define INT_UART1  0x00000004 //level 4
#define INT_TMR1   0x00000002 //level 6
#define INT_SPI2   0x00000001 //level 4

//chip names
enum{
   CHIP_BEGIN = 0,
   CHIP_A_ROM = 0,
   CHIP_B_SED,
   CHIP_C_USB,
   CHIP_D_RAM,
   CHIP_REGISTERS,
   CHIP_NONE,
   CHIP_END
};

//types
typedef struct{
   bool     enable;
   uint32_t start;
   uint32_t size;
   uint32_t mask;//the address lines the chip responds to, so 0x10000 on an chip with 16 address lines will return the value at 0x0000

   //attributes
   bool     inBootMode;
   bool     readOnly;
   bool     readOnlyForProtectedMemory;
   bool     supervisorOnlyProtectedMemory;
   uint32_t unprotectedSize;
}chip_t;

//variables
extern chip_t   chips[];
extern int32_t  pllWakeWait;
extern uint32_t clk32Counter;
extern double   timer1CycleCounter;
extern double   timer2CycleCounter;
extern uint16_t spi2ExternalData;
extern uint32_t spi2ExchangedBits;

//memory accessors
unsigned int getHwRegister8(unsigned int address);
unsigned int getHwRegister16(unsigned int address);
unsigned int getHwRegister32(unsigned int address);
void setHwRegister8(unsigned int address, unsigned int value);
void setHwRegister16(unsigned int address, unsigned int value);
void setHwRegister32(unsigned int address, unsigned int value);

//timing
void clk32();//also checks all interrupts

//CPU
bool pllIsOn();
bool registersAreXXFFMapped();
bool sed1376ClockConnected();
void refreshButtonState();
int  interruptAcknowledge(int intLevel);

//memory errors
void setBusErrorTimeOut();
void setPrivilegeViolation();
void setWriteProtectViolation();

//config
void resetHwRegisters();
void setRtc(uint32_t days, uint32_t hours, uint32_t minutes, uint32_t seconds);
