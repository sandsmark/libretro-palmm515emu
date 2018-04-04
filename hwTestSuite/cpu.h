#ifndef CPU_HEADER
#define CPU_HEADER

#include <PalmOS.h>
#include <stdint.h>


#define CPU_NONE 0x00/*only if in emulator with special features enabled, returned for physical cpu in emulator*/
#define CPU_M68K 0x01
#define CPU_ARM  0x02
#define CPU_BOTH (CPU_M68K | CPU_ARM)/*only if in emulator with special features enabled*/

#define CPU_M68K_328 0x10
#define CPU_M68K_EZ  0x20
#define CPU_M68K_VZ  0x40
#define CPU_M68K_SZ  0x80


uint8_t getPhysicalCpuType();
uint8_t getSupportedInstructionSets();
char*   getCpuString();
Boolean enterUnsafeMode();
#endif
